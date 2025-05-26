#ifndef Chessboard_Pins
#define Chessboard_Pins

// Pin Definitions for Electronic Chess Board 
// ESP32-S3-Mini-1 GPIO mapping

// TODO: this hardware button is to be replaced with gui button
constexpr int buttonPin = 4;

constexpr int ledPin = 17;

// Shift Register Pins
constexpr int latchPin = 7;
constexpr int clockEnablePin = 9;
constexpr int dataIn = 6;
constexpr int clockPin = 8;

// Board Size
constexpr int numRegisters = 8;
constexpr int numBits = numRegisters * 8;

enum PieceStatus {
  empty,
  occupied,
  potentially_captured
};

enum PieceColor {
  none,
  white,
  black
};

#endif // Chessboard_Pins

#ifndef Arm_Pins
#define Arm_Pins

// in mm
constexpr long ArmSegment1Length = 260;
constexpr long ArmSegment2Length = 170;

// X Axis motor pins
constexpr int xStepPin = 5;
constexpr int xDirPin = 4; 

// Y Axis motor pins
constexpr int yStepPin = 7;
constexpr int yDirPin = 6;

// Z Axis motor step pins
constexpr int zStepPin = 9;
constexpr int zDirPin = 8;

constexpr long stepsPerRevolution = 800;  // Adjust for quarter-stepping

// Electromagnet Control
constexpr int electromagnetPin = 38;

// Limit switch pinout constants
constexpr int xLimitPin = 10;
constexpr int yLimitPin = 11;
constexpr int zLimitPin = 12;

// reduction ratios
constexpr double BaseGearReductionRatio = 5.5;
constexpr double ArmGearReductionRatio = 22;

// default config for stepper motors
constexpr float baseStepperSpeed = 1000.0;
constexpr float baseStepperAccel = 50.0;


#endif

//#define M_PI = 3.1415