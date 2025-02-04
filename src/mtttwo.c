#include <stdio.h>
#include "gamesman.h"

POSITION gNumberOfPositions = 1162261467;
POSITION kBadPosition = -1;

POSITION gInitialPosition = 0;
POSITION gMinimalPosition = 0;

STRING kAuthorName = "Stella Wan, Nala Chen";
STRING kGameName = "Tic-Tac-Two";
BOOLEAN kPartizan = TRUE;
BOOLEAN kDebugMenu = TRUE;
BOOLEAN kGameSpecificMenu = FALSE;
BOOLEAN kTieIsPossible = TRUE;
BOOLEAN kLoopy = TRUE;
BOOLEAN kDebugDetermineValue = FALSE;
void* gGameSpecificTclInit = NULL;

STRING kHelpGraphicInterface = "";

STRING kHelpTextInterface = "";

STRING kHelpOnYourTurn = "";

STRING kHelpStandardObjective = "";

STRING kHelpReverseObjective = "";

STRING kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */ "";

STRING kHelpExample = "";

/*************************************************************************
**
** Everything above here must be in every game file
**
**************************************************************************/

/*************************************************************************
**
** Every variable declared here is only used in this file (game-specific)
**
**************************************************************************/

// NOTE: The inner grid is always 3x3
#define GRIDROWS    3
#define GRIDCOLS    3
#define BOARDROWS   4
#define BOARDCOLS   4
#define BOARDSIZE   16
// NUMPIECES: number of pieces each player has
#define NUMPIECES   4

typedef enum possibleBoardPieces {
  Blank, X, O
} BlankOX;

char *gBlankOXString[] = { " ", "x", "o" };

int g3Array[] = { 1, 3, 9, 27, 81, 243, 729, 2187, 6561, 19683, 59049, 177147, 531441, 1594323, 4782969, 14348907, 43046721, 129140163, 387420489, 1162261467 };

typedef struct {
  BlankOX board[BOARDSIZE];
  int xoffset; // Can only use two lowest bits
  int yoffset; // Can only use two lowest bits
  // No need to hash the values below...
  BlankOX nextPiece;
  int piecesXPlaced;
  int piecesOPlaced;
  int piecesPlaced;
} GameBoard;

// Unhash from Position to GameBoard
void PositionToGameBoard(POSITION thePos, GameBoard *theGameBoard);
// Hash from GameBoard to Position
POSITION GameBoardToPosition(GameBoard *theGameBoard);

BOOLEAN kSupportsSymmetries = FALSE; /* Whether we support symmetries */

/************************************************************************
**
** NAME: InitializeDatabases
**
** DESCRIPTION: Initialize the gDatabase, a global variable.
**
************************************************************************/

void InitializeGame()
{
  gInitialPosition = GetInitialPosition();
}

void FreeGame()
{
}

/************************************************************************
**
** NAME: DebugMenu
**
** DESCRIPTION: Menu used to debub internal problems. Does nothing if
** kDebugMenu == FALSE
**
************************************************************************/

void DebugMenu()
{
}

/************************************************************************
**
** NAME: GameSpecificMenu
**
** DESCRIPTION: Menu used to change game-specific parmeters, such as
** the side of the board in an nxn Nim board, etc. Does
** nothing if kGameSpecificMenu == FALSE
**
************************************************************************/

void GameSpecificMenu() {
}

/************************************************************************
**
** NAME: SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
** Ignore if you don't care about Tcl for now.
**
************************************************************************/

void SetTclCGameSpecificOptions(int theOptions[])
{
}

/************************************************************************
**
** NAME: DoMove
**
** DESCRIPTION: Apply the move to the position.
**
** INPUTS: POSITION position : The old position
** MOVE move : The move to apply.
**
** OUTPUTS: (POSITION) : The position that results after the move.
**
** CALLS: PositionToBlankOX(POSITION,*BlankOX)
** BlankOX WhosTurn(*BlankOX)
**
************************************************************************/

POSITION DoMove(POSITION position, MOVE move)
{
  // First create a gameboard given the position
  GameBoard gameboard;
  // Retrieves all the important information back into the gameboard
  PositionToGameBoard(position, &gameboard);
  // Get the type of move (0: place new pieces, 1: move existing pieces, 2: move grid)
  int type = move / 729;
  move -= type * 729;
  // Get the source index
  int source = move / 27;
  move -= source * 27;
  // Get the destination indexs
  int destination = move;
  if (type == 0) {
    // Place new piece onto the grid
    gameboard.board[destination] = gameboard.nextPiece;
    if (gameboard.nextPiece == X) {
      gameboard.piecesXPlaced += 1;
      gameboard.nextPiece = O;
    } else {
      gameboard.piecesOPlaced += 1;
      gameboard.nextPiece = X;
    }
    gameboard.piecesPlaced += 1;
    return GameBoardToPosition(&gameboard);
  } else if (type == 1) {
    // Move piece on board
    gameboard.board[destination] = gameboard.nextPiece;
    gameboard.board[source] = Blank;
    if (gameboard.nextPiece == X) {
      gameboard.nextPiece = O;
    } else {
      gameboard.nextPiece = X;
    }
    return GameBoardToPosition(&gameboard);
  } else {
    // Move grid
    if (gameboard.nextPiece == X) {
      gameboard.nextPiece = O;
    } else {
      gameboard.nextPiece = X;
    }
    int gridy = destination / BOARDROWS;
    int gridx = destination - gridy * BOARDROWS;
    gameboard.xoffset = gridx;
    gameboard.yoffset = gridy;
    return GameBoardToPosition(&gameboard);
  }
}

void UndoMove(MOVE move)
{
}

/************************************************************************
**
** NAME: GetInitialPosition
**
** DESCRIPTION: Ask the user for an initial position for testing. Store
** it in the space pointed to by initialPosition;
**
** OUTPUTS: POSITION initialPosition : The position to fill.
**
************************************************************************/

POSITION GetInitialPosition()
{
  GameBoard gameboard;
  BlankOX board[BOARDSIZE];

  memcpy(gameboard.board, board, BOARDSIZE * sizeof(BlankOX));
  // Note: x and y offset can be different for different board size
  gameboard.xoffset = 0;
  gameboard.yoffset = 0;
  gameboard.nextPiece = X;
  gameboard.piecesPlaced = 0;
  gameboard.piecesOPlaced = 0;
  gameboard.piecesXPlaced = 0;
  
  for (int i = 0; i < BOARDSIZE; i++) {
    gameboard.board[i] = Blank;
  }

  return GameBoardToPosition(&gameboard);
}

/************************************************************************
**
** NAME: PrintComputersMove
**
** DESCRIPTION: Nicely format the computers move.
**
** INPUTS: MOVE *computersMove : The computer's move.
** STRING computersName : The computer's name.
**
************************************************************************/

void PrintComputersMove(computersMove,computersName)
MOVE computersMove;
STRING computersName;
{
}

/************************************************************************
**
** NAME: Primitive
**
** DESCRIPTION: If there is a three in a row in the grid, then that is a
** losing position, else the position is undecided.
**
** INPUTS: POSITION position : The position to inspect.
**
** OUTPUTS: (VALUE) an enum which is oneof: (win,lose,tie,undecided)
**
** CALLS: BOOLEAN ThreeInARow()
** BOOLEAN PositionToGameBoard()
**
************************************************************************/

VALUE Primitive(POSITION position)
{
  // First create a gameboard given the position
  GameBoard gameboard;
  // Retrieves all the important information back into the gameboard
  PositionToGameBoard(position, &gameboard);
  // Here gameboard->xoffset and gameboard->yoffset should tell you
  // where the top left corner of the grid is
  int gridx = gameboard.xoffset;
  int gridy = gameboard.yoffset;
  // If 3 in a row horizontally
  for (int i = gridy; i < gridy + GRIDROWS; i += 1) {
    int index = gridx + i * BOARDCOLS;
    if (gameboard.board[index] == gameboard.board[index + 1]
        && gameboard.board[index + 1] == gameboard.board[index + 2]
        && gameboard.board[index] != Blank) {
          return lose;
        }
  }
  // If 3 in a row vertically
  for (int i = gridx; i < gridx + GRIDCOLS; i += 1) {
    int index = i + gridy * BOARDCOLS;
    if (gameboard.board[index] == gameboard.board[index + BOARDCOLS]
        && gameboard.board[index + BOARDCOLS] == gameboard.board[index + 2 * BOARDCOLS]
        && gameboard.board[index] != Blank) {
          return lose;
        }
  }
  // If 3 in a row diagonally with top left corner
  int index = gridx + gridy * BOARDCOLS;
  if (gameboard.board[index] == gameboard.board[index + BOARDCOLS + 1]
      && gameboard.board[index + BOARDCOLS + 1] == gameboard.board[index + 2 * BOARDCOLS + 2]
      && gameboard.board[index] != Blank) {
        return lose;
      }
  // If 3 in a row diagonally with top right corner
  index = gridx + gridy * BOARDCOLS + 2;
  if (gameboard.board[index] == gameboard.board[index + BOARDCOLS - 1]
      && gameboard.board[index + BOARDCOLS - 1] == gameboard.board[index + 2 * BOARDCOLS - 2]
      && gameboard.board[index] != Blank) {
        return lose;
      }
  return undecided;
}



/************************************************************************
**
** NAME: PrintPosition
**
** DESCRIPTION: Print the position in a pretty format, including the
** prediction of the game's outcome.
**
** INPUTS: POSITION position : The position to pretty print.
** STRING playerName : The name of the player.
** BOOLEAN usersTurn : TRUE <==> it's a user's turn.
**
** CALLS: PositionToBlankOX()
** GetValueOfPosition()
** GetPrediction()
**
************************************************************************/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn)
{
  // Create new gameboard
  GameBoard gameboard; 
  // Retrieves all the important information back into the gameboard
  PositionToGameBoard(position, &gameboard);
  // Here gameboard->xoffset and gameboard->yoffset should tell you
  // where the top left corner of the grid is
  int gridx = gameboard.xoffset;
  int gridy = gameboard.yoffset;
  printf("\n");
  for (int i = 0; i < BOARDROWS; i++) {
    // Print two rows for each row in the original board to encode for 
    // pieces (row 1) and inner grid (row 2) 
    for (int j = 0; j < BOARDCOLS; j++) {
      printf("  "); 
      int index = j + i * BOARDCOLS;
      if (gameboard.board[index] == X) {
        printf("X"); 
        printf(" |");
      } else if (gameboard.board[index] == O) {
        printf("O");
        printf(" |");
      } else {
        printf("  |"); 
      }
    }
    printf("\n");
    for (int j = 0; j < BOARDCOLS; j++) {
      printf("__"); 
      if ((i >= gridy && i < gridy + GRIDROWS) && (j >= gridx && j < gridx + GRIDCOLS)) {
        printf("#_|");
      } else {
        printf("__|");
      }
    }
    printf("\n");
  }
}

/************************************************************************
**
** NAME: GenerateMoves
**
** DESCRIPTION: Create a linked list of every move that can be reached
** from this position. Return a pointer to the head of the
** linked list.
**
** INPUTS: POSITION position : The position to branch off of.
**
** OUTPUTS: (MOVELIST *), a pointer that points to the first item
** in the linked list of moves that can be generated.
**
** CALLS: MOVELIST *CreateMovelistNode(MOVE,MOVELIST *)
**
************************************************************************/

// Generate Moves:
// If there has been fewer than 4 pieces:
// We can 1) place pieces in empty locations within the grid
// If there has been at least 4 pieces:
// We can 1) place pieces in empty locations within the grid
//        2) move the grid
//        3) place new pieces on empty locations within the grid

MOVELIST *GenerateMoves(POSITION position)
{
  // First create a gameboard given the position
  GameBoard gameboard;
  // Retrieves all the important information back into the gameboard
  PositionToGameBoard(position, &gameboard);
  // Create a linked list of possible moves
  MOVELIST *moves = NULL;
  // Figure out the player
  int numPlaced = 0;
  if (gameboard.nextPiece == X) {
    numPlaced = gameboard.piecesXPlaced;
  } else {
    numPlaced = gameboard.piecesOPlaced;
  }
  if (gameboard.piecesPlaced < 8 && numPlaced < 4) {
    // Place new pieces in grid
    int gridx = gameboard.xoffset;
    int gridy = gameboard.yoffset;
    for (int i = gridx; i < gridx + GRIDCOLS; i += 1) {
      for (int j = gridy; j < gridy + GRIDROWS; j += 1) {
        int location = i + j * BOARDCOLS;
        if (gameboard.board[location] == Blank) {
          // First 3 ternary digits: destination, next 3 ternary digits: source (0), move type (0 = placing new)
          MOVE newMove = location + 0 * 27 + 0 * 729;
          moves = CreateMovelistNode(newMove, moves);
        }
      }
    }
  }
  if (gameboard.piecesPlaced >= 4) {
    // Move grid
    int gridx = gameboard.xoffset;
    int gridy = gameboard.yoffset;
    // Move left
    if (gridx - 1 >= 0) {
      int source = gridx + gridy * BOARDCOLS;
      int destination = (gridx - 1) + gridy * BOARDCOLS;
      MOVE newMove = destination + source * 27 + 2 * 729;
      moves = CreateMovelistNode(newMove, moves);
    }
    // Move right
    if (gridx + 1 <= BOARDCOLS - GRIDCOLS) {
      int source = gridx + gridy * BOARDCOLS; 
      int destination = (gridx + 1) + gridy * BOARDCOLS; 
      MOVE newMove = destination + source * 27 + 2 * 729; 
      moves = CreateMovelistNode(newMove, moves); 
    }
    // Move up 
    if (gridy - 1 >= 0){
      int source = gridx + gridy * BOARDCOLS;  
      int destination = gridx + (gridy - 1) * BOARDCOLS; 
      MOVE newMove = destination + source * 27 + 2 * 729; 
      moves = CreateMovelistNode(newMove, moves); 
    }
    // Move down
    if (gridy + 1 <= BOARDROWS - GRIDROWS) {
      int source = gridx + gridy * BOARDCOLS;  
      int destination = gridx + (gridy + 1) * BOARDCOLS; 
      MOVE newMove = destination + source * 27 + 2 * 729; 
      moves = CreateMovelistNode(newMove, moves); 
    }
    // Move old pieces to empty locations within grid
    for (int source = 0; source < BOARDSIZE; source ++) {
      if (gameboard.board[source] == gameboard.nextPiece) {
        for (int i = gridx; i < gridx + GRIDCOLS; i += 1) {
          for (int j = gridy; j < gridy + GRIDROWS; j += 1) {
            int location = i + j * BOARDCOLS;
            if (gameboard.board[location] == Blank) {
              // First 3 ternary digits: destination, next 3 ternary digits: source (0), move type (1 = move old pieces)
              MOVE newMove = location + source * 27 + 1 * 729;
              moves = CreateMovelistNode(newMove, moves);
            }
          }
        }
      }
    }
  }
  return moves;
}

/**************************************************/
/**************** SYMMETRY FUN BEGIN **************/
/**************************************************/

/************************************************************************
**
** NAME: GetCanonicalPosition
**
** DESCRIPTION: Go through all of the positions that are symmetrically
** equivalent and return the SMALLEST, which will be used
** as the canonical element for the equivalence set.
**
** INPUTS: POSITION position : The position return the canonical elt. of.
**
** OUTPUTS: POSITION : The canonical element of the set.
**
************************************************************************/

POSITION GetCanonicalPosition(POSITION position)
{
  return position;
}

/************************************************************************
**
** NAME: DoSymmetry
**
** DESCRIPTION: Perform the symmetry operation specified by the input
** on the position specified by the input and return the
** new position, even if it's the same as the input.
**
** INPUTS: POSITION position : The position to branch the symmetry from.
** int symmetry : The number of the symmetry operation.
**
** OUTPUTS: POSITION, The position after the symmetry operation.
**
************************************************************************/

POSITION DoSymmetry(POSITION position, int symmetry)
{
  return position;
}

/**************************************************/
/**************** SYMMETRY FUN END ****************/
/**************************************************/

/************************************************************************
**
** NAME: GetAndPrintPlayersMove
**
** DESCRIPTION: This finds out if the player wanted an undo or abort or not.
** If so, return Undo or Abort and don't change theMove.
** Otherwise get the new theMove and fill the pointer up.
**
** INPUTS: POSITION *thePosition : The position the user is at.
** MOVE *theMove : The move to fill with user's move.
** STRING playerName : The name of the player whose turn it is
**
** OUTPUTS: USERINPUT : Oneof( Undo, Abort, Continue )
**
** CALLS: ValidMove(MOVE, POSITION)
** BOOLEAN PrintPossibleMoves(POSITION) ...Always True!
**
************************************************************************/

USERINPUT GetAndPrintPlayersMove(thePosition, theMove, playerName)
POSITION thePosition;
MOVE *theMove;
STRING playerName;
{
  USERINPUT ret;

	do {
		printf("%8s's move:  ", playerName);

		ret = HandleDefaultTextInput(thePosition, theMove, playerName);
		if(ret != Continue)
			return(ret);

	}
	while (TRUE);
  return Continue;
}

/************************************************************************
**
** NAME: ValidTextInput
**
** DESCRIPTION: Return TRUE iff the string input is of the right 'form'.
** For example, if the user is allowed to select one slot
** from the numbers 1-9, and the user chooses 0, it's not
** valid, but anything from 1-9 IS, regardless if the slot
** is filled or not. Whether the slot is filled is left up
** to another routine.
**
** INPUTS: STRING input : The string input the user typed.
**
** OUTPUTS: BOOLEAN : TRUE iff the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput(STRING input)
{
  return TRUE;
}

/************************************************************************
**
** NAME: ConvertTextInputToMove
**
** DESCRIPTION: Convert the string input to the internal move representation.
**
** INPUTS: STRING input : The string input the user typed.
**
** OUTPUTS: MOVE : The move corresponding to the user's input.
**
************************************************************************/

MOVE ConvertTextInputToMove(STRING input)
{
  char type = input[0]; 
  int source; 
  int dest; 
  MOVE ret;
  if (type == 'A') {
    if (input[3] != 0) {
      dest = (input[2] - '0') * 10 + (input[3] - '0');
    } else {
      dest = input[2] - '0'; 
    }
    ret = dest;
  }
  else if (type == 'M' || type == 'G') {
    // Get source
    if (input[3] != '-') {
      source = (input[2] - '0') * 10 + (input[3] - '0');
      // Get dest 
      if (input[6] != 0) {
        dest = (input[5] - '0') * 10 + (input[6] - '0');
      } else {
        dest = input[5] - '0'; 
      }
    } else {
      source = input[2] - '0'; 
      // Get dest 
      if (input[5] != 0) {
        dest = (input[4] - '0') * 10 + (input[5] - '0');
      } else {
        dest = input[4] - '0'; 
      }
    }
    if (type == 'M') {
      ret = 1 * 729 + source * 27 + dest;
    } else {
      ret = 2 * 729 + source * 27 + dest;
    }
  }
  else {
    printf("You should not be here. Something went wrong."); 
  }
  return ret; 
}


/************************************************************************
**
** NAME: MoveToString
**
** DESCRIPTION: Returns the move as a STRING
**
** INPUTS: MOVE *move : The move to put into a string.
**
** A-DEST, M-SRC-DEST, G-SRC-DEST
************************************************************************/

STRING MoveToString (MOVE theMove)
{
  // Divide by 729
  int type = theMove / 729;
  // Divide the result by 27: gives us the destination index
  theMove -= type * 729;
  // Get the source index
  int source = theMove / 27;
  theMove -= source * 27;
  // Get the destination indexs
  int destination = theMove;
  STRING typeString; 
  // 0: placing new pieces
  // message: "A-DEST"
  if (type == 0) {
    typeString = (STRING) SafeMalloc(3); 
    strcpy(typeString, "A-"); 
    STRING destString = (STRING) SafeMalloc(3);
    sprintf(destString, "%d", destination);
    strcat(typeString, destString);
  }
  // 1: moving existing pieces
  // message: "M-SRC-DEST"
  else if (type == 1) {
    typeString = (STRING) SafeMalloc(3); 
    strcpy(typeString, "M-"); 
    STRING sourceString = (STRING) SafeMalloc(3); 
    sprintf(sourceString, "%d", source); 
    STRING destString = (STRING) SafeMalloc(3); 
    sprintf(destString, "%d", destination); 
    strcat(typeString, sourceString); 
    strcat(typeString, "-"); 
    strcat(typeString, destString); 
  }
  // 2: moving the grid
  // message: "G-SRC-DEST"
  else if (type == 2) {
    typeString = (STRING) SafeMalloc(3); 
    strcpy(typeString, "G-"); 
    STRING sourceString = (STRING) SafeMalloc(3); 
    sprintf(sourceString, "%d", source); 
    STRING destString = (STRING) SafeMalloc(3); 
    sprintf(destString, "%d", destination); 
    strcat(typeString, sourceString); 
    strcat(typeString, "-"); 
    strcat(typeString, destString);   
  }
  return typeString;
}

/************************************************************************
**
** NAME: PrintMove
**
** DESCRIPTION: Print the move in a nice format.
**
** INPUTS: MOVE *theMove : The move to print.
**
************************************************************************/

void PrintMove(MOVE theMove)
{
  STRING str = MoveToString(theMove);
	printf("%s", str);
  if (str != NULL) {
    SafeFree(str);
  } else {
    printf("Passed in empty move."); 
  }
}

STRING kDBName = "";

int NumberOfOptions()
{
  return 1;
}

int getOption()
{
  return 0;
}

void setOption(int option)
{
}

POSITION ActualNumberOfPositions(int variant)
{
  return 0;
}


POSITION InteractStringToPosition(STRING board)
{
  return 0;
}

STRING InteractPositionToString(POSITION pos)
{
  return NULL;
}

STRING InteractPositionToEndData(POSITION pos)
{
  return NULL;
}

STRING InteractMoveToString(POSITION pos, MOVE mv) {
  return MoveToString(mv);
}

POSITION GameBoardToPosition(GameBoard *theGameBoard) {
  POSITION position = 0;

  for (int i = 0; i < BOARDSIZE; i++) {
    position += g3Array[i] * (theGameBoard->board[i]);
  }

  return position + g3Array[BOARDSIZE] * theGameBoard->xoffset + g3Array[BOARDSIZE+1] * theGameBoard->yoffset + g3Array[BOARDSIZE+2] * theGameBoard->nextPiece;

}

void PositionToGameBoard(POSITION thePos, GameBoard *theGameBoard) {
  // NOTE: Avoid division
  // Extract nextPiece
  theGameBoard->nextPiece = thePos / g3Array[BOARDSIZE+2];
  thePos -= g3Array[BOARDSIZE+2] * theGameBoard->nextPiece;

  // Extract yoffset
  theGameBoard->yoffset = thePos / g3Array[BOARDSIZE+1];
  thePos -= g3Array[BOARDSIZE+1] * theGameBoard->yoffset;
  
  // Extract xoffset
  theGameBoard->xoffset = thePos / g3Array[BOARDSIZE];
  thePos -= g3Array[BOARDSIZE] * theGameBoard->xoffset;

  // Get the number of pieces placed for X and O
  int numX = 0;
  int numO = 0;
  // Extract boards
  for (int i = BOARDSIZE-1; i >= 0; i--) {
    theGameBoard->board[i] = thePos / g3Array[i];
    thePos -= g3Array[i] * theGameBoard->board[i];
    if (theGameBoard->board[i] == X) {
      numX += 1;
    }
    if (theGameBoard->board[i] == O) {
      numO += 1;
    }
  }
  theGameBoard->piecesXPlaced = numX;
  theGameBoard->piecesOPlaced = numO;
  theGameBoard->piecesPlaced = numX + numO;

}
