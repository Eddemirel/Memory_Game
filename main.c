#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <time.h>
#include <stdbool.h>

// ------------------ DE1-SoC/PS2 Constants & Addresses --------------------
#define PS2_BASE        ((volatile long *) 0xFF200100)
volatile int * PS2_ptr = (int *)PS2_BASE;

#define PIXEL_BUF_CTRL  ((volatile int *) 0xFF203020)
int pixel_buffer_start; // global variable for VGA pixel buffer

#define ADDR_7SEG1      ((volatile long *) 0xFF200020)
#define ADDR_7SEG2      ((volatile long *) 0xFF200030)

// Example PS/2 key scancodes (make sure these match your keyboard)
#define KEY_UP    0x75
#define KEY_DOWN  0x72
#define KEY_LEFT  0x6B
#define KEY_RIGHT 0x74
#define KEY_ENTER 0x5A   // numeric keypad Enter often 0x5A; main Enter may differ

// ------------------------------------------------------------------------
// ===================== Function Declarations =============================
// ------------------------------------------------------------------------
int  read_PS2_arrow_or_enter();
int  game_start(short int *array);
int  select_card_with_pointer(short int *cardsColorArray);
void flip_card_pointerVersion(int index, short int *patternArray, short int *currentColor);
void close_card_pointerVersion(int index, short int *currentColor);

void clear_screen();
void plot_pixel(int x, int y, short int line_color);
void draw_square_symbol(int x1, int y1, short int colour);
void draw_card(int x1, int y1);
void draw_all_cards();

void get_card_xy(int cardIndex, int *x, int *y);
void highlight_card(int cardIndex, short int *currentColor);
void unhighlight_card(int cardIndex, short int *currentColor);

void display_score(int player, int index);
void delay_loop(int player);

void shuffle_deck(short int *array, size_t n);

// -------------- Example Data for Testing (2-row, 10-card layout) ---------
short int testPatterns[10] = {
    0xF800, // Red
    0x07E0, // Green
    0x001F, // Blue
    0xFFE0, // Yellow
    0xF81F, // Magenta
    0xF800, // Red
    0x07E0, // Green
    0x001F, // Blue
    0xFFE0, // Yellow
    0xF81F  // Magenta
};

// ------------------------------------------------------------------------
// ============================= main() ====================================
// ------------------------------------------------------------------------
int main(void)
{
    // 1. Set up the pixel buffer
    pixel_buffer_start = *PIXEL_BUF_CTRL; 
    srand(time(NULL));  
    *(PS2_ptr) = 0xFF;  // reset PS/2

    // 2. Clear screen
    clear_screen();

    // 3. Optional: Shuffle the test patterns to randomize pairs
    shuffle_deck(testPatterns, 10);

    // 4. Run the game loop (single-player pointer-based) 
    int winner = game_start(testPatterns);

    // 5. End
    if (winner == 0) {
        printf("Player 1 won!\n");
    } else {
        printf("Player 2 won!\n");
    }

    // Just loop forever for this demo
    while(1);
    return 0;
}

// ------------------------------------------------------------------------
// ==================== game_start() Using Pointer =========================
// ------------------------------------------------------------------------
int game_start(short int *array)
{
    // Clear screen & draw 2 rows of "cards" (closed = white rectangles)
    clear_screen();
    draw_all_cards();

    // For demonstration, we’ll track two players and do a max total of 5 matches
    int player = 0;
    int player1_score = 0;
    int player2_score = 0;
    int index1 = -1, index2 = -1;
    int score = 0;

    // Keep track of which cards are matched
    bool matched[10] = {false, false, false, false, false, 
                        false, false, false, false, false};

    // Keep track of the "current color" on each card (white if closed)
    short int currentColor[10];
    for (int i = 0; i < 10; i++) {
        currentColor[i] = 0xFFFF; // all start closed (white)
    }

    printf("Player 1's turn!\n");

    // We have 5 pairs => the game ends when 5 matches are found
    while ( (player1_score + player2_score) < 5 ) 
    {
        // 1) Select the first card
        bool valid = false;
        while(!valid) {
            index1 = select_card_with_pointer(currentColor);
            if (!matched[index1]) {
                valid = true;
            } else {
                printf("Card %d is already matched. Choose another.\n", index1);
            }
        }
        // flip open the first card
        flip_card_pointerVersion(index1, array, currentColor);

        // 2) Select the second card
        valid = false;
        while(!valid) {
            index2 = select_card_with_pointer(currentColor);
            if (!matched[index2] && index1 != index2) {
                valid = true;
            } else {
                printf("Invalid or already matched. Choose again.\n");
            }
        }
        // flip open the second card
        flip_card_pointerVersion(index2, array, currentColor);

        // 3) Check match
        if (array[index1] == array[index2]) {
            // match
            matched[index1] = true;
            matched[index2] = true;
            score = 1;
            if (player == 0) {
                printf("Player 1 gets a point.\n");
                printf("Player 2's turn!\n");
            } else {
                printf("Player 2 gets a point.\n");
                printf("Player 1's turn!\n");
            }
        } else {
            // mismatch => delay, then flip them back
            if (player == 0) {
                printf("Not a match. Player 1 doesn't score.\n");
            } else {
                printf("Not a match. Player 2 doesn't score.\n");
            }
            delay_loop(player);

            close_card_pointerVersion(index1, currentColor);
            close_card_pointerVersion(index2, currentColor);
        }

        // 4) Update scores
        if (player == 0) {
            player1_score += score;
            display_score(player, player1_score);
            player = 1;
        } else {
            player2_score += score;
            display_score(player, player2_score);
            player = 0;
        }
        score = 0;
    }

    // The loop ended => someone matched 5 pairs 
    if (player1_score > player2_score) {
        return 0;  // Player 1 wins
    }
    return 1;      // Player 2 wins
}

// ------------------------------------------------------------------------
// ==================== Pointer-Based Selection ============================
// ------------------------------------------------------------------------

// Reads PS/2 data until we get arrow or ENTER
int read_PS2_arrow_or_enter()
{
    int PS2_data, RVALID;
    char byte = 0;
	int f_count = 0;

    while(1) {
        PS2_data = *(PS2_ptr);
        RVALID   = PS2_data & 0x8000;
		if (RVALID){printf("RVALID: %d\n", RVALID);};
        if (RVALID) {
            byte = PS2_data & 0xFF;
			
            // Some keyboards send 0xE0 or 0xF0 before arrow keys
            if (byte == 0xE0) {
                continue; 
            }
			else if (byte == 0xF0){
				f_count += 1;
				continue;
			}
            // Check if arrow or Enter
            if (byte == KEY_UP   || byte == KEY_DOWN  || 
                byte == KEY_LEFT || byte == KEY_RIGHT ||
                byte == KEY_ENTER) {
				if (f_count == 1){
					f_count = 0;
                	return byte;}
            }
        }
    }
}

// Let user move pointer among 10 cards (2 rows x 5 cols)
int select_card_with_pointer(short int *cardsColorArray)
{
    static int currentPointerIndex = 0; 
    // You can reset to 0 if you want each selection to start at top-left:
    //   int currentPointerIndex = 0;

    // highlight initial
    highlight_card(currentPointerIndex, cardsColorArray);

    while(1) {
        int key = read_PS2_arrow_or_enter();

        // unhighlight old pointer
        unhighlight_card(currentPointerIndex, cardsColorArray);

        if (key == KEY_LEFT) {
            // move pointer left if not at leftmost col
            if (currentPointerIndex % 5 > 0) {
                currentPointerIndex--;
            }
        }
        else if (key == KEY_RIGHT) {
            // move pointer right if not at rightmost col
            if (currentPointerIndex % 5 < 4) {
                currentPointerIndex++;
            }
        }
        else if (key == KEY_UP) {
            // move pointer up if in bottom row
            if (currentPointerIndex >= 5) {
                currentPointerIndex -= 5;
            }
        }
        else if (key == KEY_DOWN) {
            // move pointer down if in top row
            if (currentPointerIndex < 5) {
                currentPointerIndex += 5;
            }
        }
        else if (key == KEY_ENTER) {
            // re-highlight so we can see the final position
            highlight_card(currentPointerIndex, cardsColorArray);
            return currentPointerIndex;
        }

        // highlight new pointer
        highlight_card(currentPointerIndex, cardsColorArray);
    }
}

// Flip card open
void flip_card_pointerVersion(int index, short int *patternArray, short int *currentColor)
{
    int x, y;
    get_card_xy(index, &x, &y);
    draw_square_symbol(x, y, patternArray[index]);
    currentColor[index] = patternArray[index]; 
}

// Flip card closed (white)
void close_card_pointerVersion(int index, short int *currentColor)
{
    int x, y;
    get_card_xy(index, &x, &y);
    draw_square_symbol(x, y, 0xFFFF);  // white
    currentColor[index] = 0xFFFF;
}

// ------------------------------------------------------------------------
// ====================== Pointer Highlighting =============================
// ------------------------------------------------------------------------

// Convert card index (0..9) to on-screen (x,y)
void get_card_xy(int cardIndex, int *x, int *y)
{
    int row = cardIndex / 5; // 0 => top row, 1 => bottom row
    int col = cardIndex % 5;

    *x = 30 + col * 59; 
    if (row == 0) {
        *y = 56; 
    } else {
        *y = 160;
    }
}

// Draw a red overlay
void highlight_card(int cardIndex, short int *currentColor)
{
    int x, y;
    get_card_xy(cardIndex, &x, &y);
    // "Overlay" in red.  In a real game, you might draw a red rectangle border 
    // instead. For simplicity, we overwrite the 24x24 area:
    draw_square_symbol(x, y, 0xF800);  // red

    // Then, you could draw a smaller area inside with currentColor[cardIndex]
    // if you want to preserve the pattern. 
    // For a simpler approach, we’ll just show the big red block 
    // while pointer is here.
}

// Remove highlight by re-drawing the card’s current color
void unhighlight_card(int cardIndex, short int *currentColor)
{
    int x, y;
    get_card_xy(cardIndex, &x, &y);
    draw_square_symbol(x, y, currentColor[cardIndex]);
}

// ------------------------------------------------------------------------
// =========================== Utility Routines ============================
// ------------------------------------------------------------------------
void shuffle_deck(short int *array, size_t n)
{
    if (n > 1) {
        for (size_t i = 0; i < n - 1; i++) {
            size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
            short int t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}

// Clear entire 320x240 screen to black
void clear_screen()
{
    for(int x = 0; x < 320; x++){
        for(int y = 0; y < 240; y++){
            plot_pixel(x, y, 0x0000); // black
        }
    }
}

void plot_pixel(int x, int y, short int line_color)
{
    volatile short int *pixel_ptr;
    pixel_ptr = (volatile short int *)(pixel_buffer_start + (y << 10) + (x << 1));
    *pixel_ptr = line_color;
}

// Draw a 24x24 square
void draw_square_symbol(int x1, int y1, short int colour)
{
    for(int x = x1; x < x1 + 24; x++){
        for(int y = y1; y < y1 + 24; y++){
            plot_pixel(x, y, colour);
        }
    }
}

// Draw a 34x72 white card area (just like in your original code)
void draw_card(int x1, int y1)
{
    for(int x = x1; x < x1 + 34; x++){
        for(int y = y1; y < y1 + 72; y++){
            plot_pixel (x, y, 0xFFFF); // white 
        }
    }
}

// Draw 10 "white cards": 2 rows x 5 columns
void draw_all_cards()
{
    // bottom row
    draw_card(25, 136);
    draw_card(84, 136);
    draw_card(143, 136);
    draw_card(202, 136);
    draw_card(261, 136);
    
    // top row
    draw_card(25, 32);
    draw_card(84, 32);
    draw_card(143, 32);
    draw_card(202, 32);
    draw_card(261, 32);
}

// Display a tiny score on the HEX for demonstration
void display_score(int player, int index)
{
    // Example array for hex digits 0..5
    int scoreHex[6] = {
        0b00111111, // 0
        0b00000110, // 1
        0b01011011, // 2
        0b01001111, // 3
        0b01100110, // 4
        0b01101101, // 5
    };
    // clamp index to 0..5
    if (index < 0) index = 0;
    if (index > 5) index = 5;

    if (player == 0) {
        *ADDR_7SEG1 = scoreHex[index];
    } else {
        *ADDR_7SEG2 = scoreHex[index];
    }
}

// Simple delay + next-player prompt
void delay_loop(int player)
{
    int i = 1000000;
    while(i--) { /* spin */ }

    if (player == 1) {
        printf("Player 1's turn!\n");
    } else {
        printf("Player 2's turn!\n");
    }
}
