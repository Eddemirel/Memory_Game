Memory Game 
General configurations (alway enabled)
Press esc to quit (quit must take us to set up screen)

CARDS:
B mode has 12 cards - 6 pairs(4x3), M mode has 20 cards 10 pairs (4x5) and D mode has 32 cards 16 pairs (4x8) (rows x columns)
We will have 16 pairs of unique card patterns for open faces
Each game selects the necessary number of pairs randomly, and then allocates cards to the game grid randomly.
Background is black and the closed faces are all white. Cards are laid on the background closed unless they are selected to be open.

TURN:
Opening 2 cards means 1 turn
Each turn starts with the the card pointer pointing to left-top card. 
The user should be able to change the card pointer location using up down left right arrows to select a card.
The card that the card pointer points turns red.
To be able to open 2 cards in a turn: User must select the first card by navigating with the arrows of the keyboard, changing the location of the card pointer and observing the red selection. When the desired card is pointed, the user must hit enter, this card stays red until the second card is selected. Then the user continues to move the card pointer with arrows again, and again the pointed card turns red (now we have one constant and one moving red card). User must select a different card then the first selected one, and hit enter. Then 2 selected cards will display their open faces.
If 2 cards are a pair, then play clap audio, and these cards will remain open for the rest of the game.
If they are not then both will turn their back sides again (i.e. white)
This concludes a turn. 


Set up screen 
We need to be able to configure game settings
player mode (1 or 2 players)
Difficulty mode (basic, medium, hard)
We can press 1 or 2 to select user mode then hit enter to confirm (can be 1 by default)
We can press B, M or H to select difficulty mode and hit enter to confirm (can be B by default)
Regardless we made selection of player and difficulty, when we hit space, the game starts
These selections should also be displayed on the screen in a user friendly way.
This screen should not display cards yet. we need to see cards with their closed faces after we hit space.


Game (single) player
As soon as we hit the space to start the game, we need to start a timer to count seconds and display time on the seven segment display until the time finishes.
Player must continue his/her turns as long as all the cards are paired.
When the game is completed, stay with all the cards open and the timer is stopped, play the game's ringtone.
Press esc to quit (quit must take us to set up screen)

Game (2 players)
When we hit the space to start the game, we need to initialize the seven segment display with 100200.
For the 2 player mode 6 seven segments we will display 1xx2yy where xx is player 1 score, and yy is player2 mode.
User 1 uses left right up down arrows to navigate whereas user 2 uses letters A for loft, S for down, D for right and W for up. 
Game starts with a user 1 turn.
If user 1 finds a card pair, we will increment the score on the seven segment and it is again a user 1 turn, else turn is user 2’s.
Game continues till all card pairs are found. 
When the game is completed, stay with all the cards open and the timer is stopped, play the game's ringtone.
Press esc to quit (quit must take us to set up screen)

