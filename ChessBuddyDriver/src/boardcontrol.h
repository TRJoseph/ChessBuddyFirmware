#ifndef BOARDCONTROL
#define BOARDCONTROL

#include <Arduino.h>


#define MAX_MOVES 200
#define MOVE_LENGTH 6


// holding current status of calibration
extern bool calibrationStatus;

// true if it is the user's side to move, else robot's turn
extern bool userSideToMove;

// true if a game has started
extern bool activeGame;

extern char moveHistory[MAX_MOVES][MOVE_LENGTH];
extern int moveCount;

// for board piece detection
extern int potentialMovedFromSquare;
extern int potentialMovedToSquare;
extern int numPiecesPickedUp;
extern bool captureMove;

enum class SpecialMove {
    None,
    Castling, 
    Capture,
    EnPassant, 
    Promotion
};

enum class PieceType {
    Pawn,
    Bishop,
    Knight,
    Rook,
    Queen,
    King
};

enum SquareStatus {
    Empty,
    Occupied,
    Potentially_Captured
};

enum SquareColor {
    None,
    White,
    Black
};

struct KeyValuePair {
    PieceType key;
    int value;
};


/* FUNCTION DEFINITIONS */

void runCalibrationRoutine();
void gotoParkPosition();
int* getSquarePosition(const String& square);
void performQuietMove(String moveString, PieceType pieceType, SpecialMove specialMove);
void performKingSideCastle(String moveString);
void performQueenSideCastle(String moveString);
void performCaptureMove(String moveString, PieceType pieceType, PieceType capturedPieceType, SpecialMove specialMove);
void moveToSquare(char square[]);
int getPieceZOffset(PieceType key);
PieceType stringToPieceType(const char* pieceStr);
void inverseKinematics(long x, long y);
int splitString(String input, char delimiter, String outputArray[]);
void editSquareStates(uint8_t fromSquare, uint8_t toSquare);
void algebraicToSquares(const char move[], uint8_t& fromSquare, uint8_t& toSquare);
void processCommand(String input);
void handleArmMove(const char* move);
void instantiateBoardState();
void updateCurrentBoardState();
void resetPieceDetectionParameters();
uint64_t readShiftRegisters();
String squareNumToAlgebraic(int square);
String combineSquareStrings(int fromSquare, int toSquare);
void scanningUserMove(bool isUserSideToMove, bool isFinalizedMove);
void deduceUserMove();
void printMoveHistory();
void boardStartNewGame();
void setupBoard();

#endif