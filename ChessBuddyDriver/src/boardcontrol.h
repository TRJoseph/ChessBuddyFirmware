#include <Arduino.h>

struct Square {
    byte status; // 0=empty, 1=occupied, 2=potentially_captured
    byte color;  // 0=none, 1=white, 2=black
};

Square squareStates[64];

// Chess board x,y positions relative to the robotic arm
// bottom left square is index 0, top right square is index 63
int SquarePositions[64][2] = {
  // 1st rank
  {-135, 102}, 
  {-97, 102},
  {-60, 102},
  {-22, 102},
  {18, 102},
  {59, 102},
  {97, 102},
  {135, 102},
  // 2nd rank
  {-135, 140}, 
  {-97, 140},
  {-60, 140},
  {-22, 140},
  {18, 140},
  {59, 140},
  {97, 140},
  {135, 140},
  // 3rd rank
  {-135, 182}, 
  {-97, 182},
  {-60, 182},
  {-22, 182},
  {18, 182},
  {59, 182},
  {97, 182},
  {135, 182},
  // 4th rank
  {-135, 220}, 
  {-97, 220},
  {-60, 220},
  {-22, 220},
  {18, 220},
  {59, 220},
  {97, 220},
  {135, 220},
  // 5th rank
  {-135, 258}, 
  {-97, 258},
  {-60, 258},
  {-22, 258},
  {18, 258},
  {59, 258},
  {97, 258},
  {135, 258},
  // 6th rank
  {-135, 297}, 
  {-97, 297},
  {-60, 297},
  {-22, 297},
  {18, 297},
  {59, 297},
  {97, 297},
  {135, 297},
  // 7th rank
  {-135, 336}, 
  {-97, 336},
  {-60, 336},
  {-22, 336},
  {18, 336},
  {59, 336},
  {97, 336},
  {135, 336},
  // 8th rank
  {-135, 375}, 
  {-97, 375},
  {-60, 375},
  {-22, 375},
  {18, 375},
  {59, 375},
  {97, 375},
  {135, 375},
};

Square currentBoardState[64];

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

// array of key value pairs for each piece offset
// this array is necessary because each physical piece has a different height on the chess board, the robot needs to compensate for each of those appropriately
KeyValuePair PieceZAxisOffsets[] = {
    {PieceType::Pawn, -3740},
    {PieceType::Knight, -2980},
    {PieceType::Bishop, -2500},
    {PieceType::Rook, -3370},
    {PieceType::Queen, -1940},
    {PieceType::King, -650} // king is good
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