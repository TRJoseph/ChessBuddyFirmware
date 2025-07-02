#ifndef BOARDCONTROL
#define BOARDCONTROL

#include <Arduino.h>

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
void moveToSquare(const String& square);
int getPieceZOffset(PieceType key);
PieceType stringToPieceType(const String& pieceStr);
void inverseKinematics(long x, long y);
int splitString(String input, char delimiter, String outputArray[]);
void editSquareStates(int fromSquare, int toSquare);
void algebraicToSquares(const String& move, int& fromSquare, int& toSquare);
void processCommand(String input);
void instantiateBoardState();
uint64_t readShiftRegisters();
String squareNumToAlgebraic(int square);
String combineSquareStrings(int fromSquare, int toSquare);
void deduceUserMove();
void startNewGame();
void setupBoard();

#endif