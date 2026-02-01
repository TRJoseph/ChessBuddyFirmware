#ifndef MAINCONTROL
#define MAINCONTROL

#include <Arduino.h>
#include <unordered_map>
#include <FastAccelStepper.h>
#include <string.h>
#include "Main_Definitions.h"
#include "serverInterface.h"
#include "gui.h"
#include "gui_gateway.h"
#include "led_controller.h"


class Main_Controller {
private:
    static constexpr const char* startFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    static constexpr int MAX_MOVES = 200;
    static constexpr int MOVE_LENGTH = 6;

    // holding current status of calibration
    bool calibrationStatus = false;

    // true if it is the user's side to move, else robot's turn
    bool userSideToMove = false;

    // true if a game has started
    bool activeGame = false;

    char moveHistory[MAX_MOVES][MOVE_LENGTH];
    int moveCount = 0;

    // for board piece detection
    int potentialMovedFromSquare;
    int potentialMovedToSquare;
    int numPiecesPickedUp;
    bool captureMove;

    // stepper motor configuration
    float referenceStepperSpeed;
    float referenceStepperAccelScalar;

    const uint16_t maxStepperSpeed = 2000;
    const uint16_t minStepperSpeed = 1000;
    const uint8_t maxStepperAccel = 4;
    const uint8_t minStepperAccel = 1;

    int zAxisTopHeight = 8300;
    int zAxisReferenceHeight = 5800;

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

    struct Square {
        SquareStatus status; // 0=empty, 1=occupied, 2=potentially_captured
        SquareColor color;  // 0=none, 1=white, 2=black
    };

    Square squareStates[64];
    Square currentBoardState[64];

    // Chess board x,y positions relative to the robotic arm
    // bottom left square is index 0, top right square is index 63
    static constexpr int SquarePositions[64][2] = {
    // 1st rank
    {-135, 102}, {-97, 102}, {-60, 102}, {-22, 102},
    {18, 102}, {59, 102}, {97, 102}, {135, 102},
    // 2nd rank
    {-135, 140}, {-97, 140}, {-60, 140}, {-22, 140},
    {18, 140}, {59, 140}, {97, 140}, {135, 140},
    // 3rd rank
    {-135, 182}, {-97, 182}, {-60, 182}, {-22, 182},
    {18, 182}, {59, 182}, {97, 182}, {135, 182},
    // 4th rank
    {-135, 220}, {-97, 220}, {-60, 220}, {-22, 220},
    {18, 220}, {59, 220}, {97, 220}, {135, 220},
    // 5th rank
    {-135, 258}, {-97, 258}, {-60, 258}, {-22, 258},
    {18, 258}, {59, 258}, {97, 258}, {135, 258},
    // 6th rank
    {-135, 297}, {-97, 297}, {-60, 297}, {-22, 297},
    {18, 297}, {59, 297}, {97, 297}, {135, 297},
    // 7th rank
    {-135, 336}, {-97, 336}, {-60, 336}, {-22, 336},
    {18, 336}, {59, 336}, {97, 336}, {135, 336},
    // 8th rank
    {-135, 375}, {-97, 375}, {-60, 375}, {-22, 375},
    {18, 375}, {59, 375}, {97, 375}, {135, 375},
    };

    // array of key value pairs for each piece offset
    // this array is necessary because each physical piece has a different height on the chess board, the robot needs to compensate for each of those appropriately
    std::unordered_map<PieceType, int> PieceZAxisOffsets = {
        {PieceType::Pawn, -4760},
        {PieceType::Knight, -4000},
        {PieceType::Bishop, -3520},
        {PieceType::Rook, -4390},
        {PieceType::Queen, -2944},
        {PieceType::King, -1670} // king is good
    };

    FastAccelStepperEngine stepperEngine = FastAccelStepperEngine();

public:

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