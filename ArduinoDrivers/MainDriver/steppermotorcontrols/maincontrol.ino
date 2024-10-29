#include <AccelStepper.h>
#include <string.h>
#define M_PI 3.14159265358979323846

// in mm
const long ArmSegment1Length = 260;
const long ArmSegment2Length = 170;

// X Axis step pins
const int xStepPin = 2;
const int xDirPin = 5;

// Y Axis step pins
const int yStepPin = 3;
const int yDirPin = 6;

// Z Axis step pins
const int zStepPin = 4;
const int zDirPin = 7;

const long stepsPerRevolution = 800;  // Adjust for quarter-stepping

// Electromagnet Control
const int electromagnetPin = 12;

// Chess board x,y positions relative to the robotic arm
// bottom left square is index 0, top right square is index 63
int SquarePositions[64][2] = {
  // 1st rank
  {-134,105.5}, 
  {-96,105.5},
  {-59,105.5},
  {-21.5,105.5},
  {17,106},
  {58,106},
  {96, 106},
  {134, 107},
  // 2nd rank
  {-134,143.5}, 
  {-96,143.5},
  {-59,143.5},
  {-21.5,145.5},
  {17,146},
  {58,146},
  {96,146},
  {134,146},
  // 3rd rank
  {-134,183.5}, 
  {-96,183.5},
  {-59,183.5},
  {-21.5,183.5},
  {17,184},
  {58,184},
  {96,184},
  {134,184},
  // 4th rank
  {-134,221.5}, 
  {-96,221.5},
  {-59,221.5},
  {-21.5,221.5},
  {17,222},
  {58,222},
  {96,222},
  {134,222},
  // 5th rank
  {-134,259.5}, 
  {-96,259.5},
  {-59,259.5},
  {-21.5,259.5},
  {17,260},
  {58,260},
  {96,260},
  {134,260},
  // 6th rank
  {-134,297.5}, 
  {-96,297.5},
  {-59,297.5},
  {-21.5,297.5},
  {17,298},
  {58,298},
  {96,298},
  {134,298},
  // 7th rank
  {-134,335.5}, 
  {-96,335.5},
  {-59,335.5},
  {-21.5,335.5},
  {17,336},
  {58,336},
  {96,336},
  {134,336},
  // 8th rank
  {-134,373.5}, 
  {-96,373.5},
  {-59,373.5},
  {-21.5,373.5},
  {17,374},
  {58,374},
  {96,374},
  {134,374},
};

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
    {PieceType::Pawn, -4280},
    {PieceType::Knight, -2500},
    {PieceType::Bishop, -2890},
    {PieceType::Rook, -3100},
    {PieceType::Queen, -2100},
    {PieceType::King, -780}
};

/* */
// Limit switch pinout constants
const int xLimitPin = 9;
const int yLimitPin = 10;
const int zLimitPin = 11;
/* */

const double BaseGearReductionRatio = 5.5;
const double ArmGearReductionRatio = 27.5;

const float baseStepperSpeed = 1000.0;
const float baseStepperAccel = 50.0;


/* THINGS TO NOTE:
The X stepper motor is referring to the base joint rotation. The Y stepper motor is referring to the arm segment joint rotation.
*/
class StepperMotor {
public:
    AccelStepper motor;
    float normalMaxSpeed;
    float normalAcceleration;
    float calibrationMaxSpeed;
    float calibrationAcceleration;

    StepperMotor(int stepPin, int dirPin, float normalSpeed, float normalAccel, float calibSpeed, float calibAccel) : motor(AccelStepper::DRIVER, stepPin, dirPin) {
        normalMaxSpeed = normalSpeed;
        normalAcceleration = normalAccel;
        calibrationMaxSpeed = calibSpeed;
        calibrationAcceleration = calibAccel;
    }

    void setNormalMotorSettings() {
        motor.setMaxSpeed(normalMaxSpeed);
        motor.setAcceleration(normalAcceleration);
    }

    void calibrate(int limitSwitchPin) {
        motor.setMaxSpeed(calibrationMaxSpeed);
        motor.setAcceleration(calibrationAcceleration);
        
        motor.moveTo(-30000);  // Move far in reverse
  
        while (digitalRead(limitSwitchPin) == HIGH) {
          motor.run();    // continue running until switch is triggered
        }

        motor.setCurrentPosition(0); // Set zero position after calibration

        // reset speed and accel settings
        setNormalMotorSettings();

        delay(2000);
    }
    
    void moveTo(long position) {
        motor.moveTo(position);
        while (motor.run()) { }
    }

    void moveToNoRun(long position) {
      // moves relative to current position
        motor.move(position);
    }
    
};

// x step pin, y dir pin, normal speed, normal acceleration, calibration speed, calibration acceleration
StepperMotor xStepperMotor(xStepPin, xDirPin, baseStepperSpeed, 1000, baseStepperSpeed / 10, baseStepperAccel / 2);
StepperMotor yStepperMotor(yStepPin, yDirPin, baseStepperSpeed * 3, 1000, 1250.0 , baseStepperAccel);
StepperMotor zStepperMotor(zStepPin, zDirPin, baseStepperSpeed * 3, 10000, baseStepperSpeed, baseStepperAccel); 

AccelStepper xStepper(AccelStepper::DRIVER, xStepPin, xDirPin);
AccelStepper yStepper(AccelStepper::DRIVER, yStepPin, yDirPin);
AccelStepper zStepper(AccelStepper::DRIVER, zStepPin, zDirPin);

void runCalibrationRoutine() {

  // ensure arm clearance for y axis calibration routine
  zStepperMotor.setNormalMotorSettings();
  zStepperMotor.moveTo(3500);

  xStepperMotor.calibrate(xLimitPin);
  xStepperMotor.moveTo(560);

  yStepperMotor.calibrate(yLimitPin);
  yStepperMotor.moveTo(11485);

  zStepperMotor.calibrate(zLimitPin);
  zStepperMotor.moveTo(9500);
  xStepperMotor.moveTo(1570);
  
}

int* getSquarePosition(const String& square) {
    // maps the square identifiers to their corresponding array indices
    int index = 0;
    char file = square.charAt(0); // 'a' to 'h'
    char rank = square.charAt(1); // '1' to '8'

    index = (rank - '1') * 8 + (file - 'a');

    if (index >= 0 && index < 64) {
        return SquarePositions[index];
    } else {
        return nullptr; // returns nullptr if the square is invalid
    }
}

// a quiet move in chess is defined as a move that does not change the current material on the board (not a capture move)
// if no special move was performed assume its quiet
void performQuietMove(String moveString, PieceType pieceType = PieceType::King, SpecialMove specialMove = SpecialMove::None) {

  size_t length = moveString.length();
  size_t midpoint = length / 2;

  int pieceZOffset = getPieceZOffset(pieceType);
  
  // split the move string into two individual squares
  String fromSquare = moveString.substring(0, midpoint);
  String toSquare = moveString.substring(midpoint);  

  moveToSquare(fromSquare);

  //move down and trigger magnet high
  zStepperMotor.moveTo(pieceZOffset);
  digitalWrite(electromagnetPin, HIGH);

  // TODO: THIS "6000" NEEDS TO BE TWEAKED, for example: pawns do not need to be lifted as high to ensure clearance over every other piece
  zStepperMotor.moveTo(7000 + pieceZOffset);

  moveToSquare(toSquare);

  zStepperMotor.moveTo(pieceZOffset);
  digitalWrite(electromagnetPin, LOW);
  zStepperMotor.moveTo(0);

  // go to park position
  inverseKinematics(-100, 0);
}

// special move here could be for en passant or a capture promotion move
void performCaptureMove(String moveString, PieceType pieceType = PieceType::King, PieceType capturedPieceType = PieceType::King, SpecialMove specialMove = SpecialMove::None) {
  size_t length = moveString.length();
  size_t midpoint = length / 2;

  int pieceZOffset = getPieceZOffset(pieceType);
  int capturedPieceZOffset = getPieceZOffset(capturedPieceType);
  
  // split the move string into two individual squares
  String fromSquare = moveString.substring(0, midpoint);
  String toSquare = moveString.substring(midpoint);  

  // extra steps for capture, we need to visit the "to square" first
  moveToSquare(toSquare);

  //move down and trigger magnet high
  zStepperMotor.moveTo(capturedPieceZOffset);
  digitalWrite(electromagnetPin, HIGH);
  zStepperMotor.moveTo(8000 + capturedPieceZOffset);
  inverseKinematics(250, 0); // TODO: move to consistent location off the board
  
  zStepperMotor.moveTo(capturedPieceZOffset);
  digitalWrite(electromagnetPin, LOW);
  zStepperMotor.moveTo(0);

  moveToSquare(fromSquare);

  //move down and trigger magnet high
  zStepperMotor.moveTo(pieceZOffset);
  digitalWrite(electromagnetPin, HIGH);

  zStepperMotor.moveTo(7000 + pieceZOffset);

  moveToSquare(toSquare);

  zStepperMotor.moveTo(pieceZOffset);
  digitalWrite(electromagnetPin, LOW);
  zStepperMotor.moveTo(0);

  // go to park position
  inverseKinematics(-100, 0);
}

void moveToSquare(const String& square) {
  int* position = getSquarePosition(square);
  if (position != nullptr) {
      inverseKinematics(position[0], position[1]);
      delay(50);
  }
}

int getPieceZOffset(PieceType key) {
    for (int i = 0; i < sizeof(PieceZAxisOffsets) / sizeof(PieceZAxisOffsets[0]); i++) {
        if (PieceZAxisOffsets[i].key == key) {
            return PieceZAxisOffsets[i].value;
        }
    }
    return -1;  // return -1 if the key is not found 
}

PieceType stringToPieceType(const String& pieceStr) {
  if (pieceStr.equalsIgnoreCase("pawn")) {
    return PieceType::Pawn;
  } else if (pieceStr.equalsIgnoreCase("bishop")) {
    return PieceType::Bishop;
  } else if (pieceStr.equalsIgnoreCase("knight")) {
    return PieceType::Knight;
  } else if (pieceStr.equalsIgnoreCase("rook") || pieceStr.equalsIgnoreCase("castle")) {
    return PieceType::Rook;
  } else if (pieceStr.equalsIgnoreCase("queen")) {
    return PieceType::Queen;
  } else if (pieceStr.equalsIgnoreCase("king")) {
    return PieceType::King;
  } else {
    Serial.println("Error: Invalid piece type.");
    return PieceType::King;
  }
}

void inverseKinematics(long x, long y) {
  // y motor corresponds to q2
  // x motor (base motor) corresponds to q1

  long x_squared = x * x;
  long y_squared = y * y;
  long a1_squared = ArmSegment1Length * ArmSegment1Length;
  long a2_squared = ArmSegment2Length * ArmSegment2Length;
  long denominator = 2 * ArmSegment1Length * ArmSegment2Length;

  // Calculate the cosine value
  double cosineValue = (x_squared + y_squared - a1_squared - a2_squared) / (double)denominator;

  double q2 = acos(cosineValue);
  double q1 = atan2(y, x) - atan2(ArmSegment2Length * sin(q2), ArmSegment1Length + ArmSegment2Length * cos(q2));

  // convert to degrees
  q1 = 90 - q1 * (180 / M_PI);
  q2 = q2 * (180 / M_PI);

  long targetYSteps = int((q2 * stepsPerRevolution * ArmGearReductionRatio) / 360);
  long targetXSteps = int((q1 * stepsPerRevolution * BaseGearReductionRatio) / 360);


  Serial.print("targetXSteps: ");
  Serial.println(targetXSteps);

  Serial.print("targetYSteps: ");
  Serial.println(targetYSteps);

  long yStepperPos = yStepperMotor.motor.currentPosition();
  long xStepperPos = xStepperMotor.motor.currentPosition();

  long ySteps = -(targetYSteps - abs(yStepperPos)); 

  long xSteps;

  xSteps = targetXSteps - xStepperPos;
  if ((xStepperPos < 0 && targetXSteps > 0) || (xStepperPos > 0 && targetXSteps < 0)) {
      // uses offset if we're crossing between quadrant 1 and quadrant 2
      xSteps = targetXSteps + (-xStepperPos);
  }

  Serial.print("x stepper current position: ");
  Serial.println(xStepperPos);

  Serial.print("y stepper current position: ");
  Serial.println(yStepperPos);

  Serial.print("actual X Steps: ");
  Serial.println(xSteps);
  Serial.print("actual Y Steps: ");
  Serial.println(ySteps);


  xStepperMotor.moveToNoRun(xSteps);
  yStepperMotor.moveToNoRun(ySteps);
  
  while (xStepperMotor.motor.distanceToGo() != 0 || 
         yStepperMotor.motor.distanceToGo() != 0) {
    
    xStepperMotor.motor.run();
    yStepperMotor.motor.run();
  }
}

void setup() {
  // set electromagnet pin out
  pinMode(electromagnetPin, OUTPUT);
  digitalWrite(electromagnetPin, LOW);

  // specify motor pin as output pins
  pinMode(xStepPin, OUTPUT);
  pinMode(xDirPin, OUTPUT);
  pinMode(yStepPin, OUTPUT);
  pinMode(yDirPin, OUTPUT);
  pinMode(zStepPin, OUTPUT);
  pinMode(zDirPin, OUTPUT);

  // specify limit switch pin outs for normally open operations
  pinMode(xLimitPin, INPUT_PULLUP); 
  pinMode(yLimitPin, INPUT_PULLUP); 
  pinMode(zLimitPin, INPUT_PULLUP);

  // calibrate all X,Y,Z starting positions
  runCalibrationRoutine();

  xStepperMotor.setNormalMotorSettings();
  yStepperMotor.setNormalMotorSettings();

  xStepperMotor.motor.setCurrentPosition(0);
  yStepperMotor.motor.setCurrentPosition(0);
  zStepperMotor.motor.setCurrentPosition(0);

  // start serial communication
  Serial.begin(9600);
}

int splitString(String input, char delimiter, String outputArray[]) {
  int tokenIndex = 0;
  int startIndex = 0;
  int delimiterIndex = input.indexOf(delimiter);

  while (delimiterIndex != -1) {
    outputArray[tokenIndex++] = input.substring(startIndex, delimiterIndex);
    startIndex = delimiterIndex + 1;
    delimiterIndex = input.indexOf(delimiter, startIndex);

    // avoids exceeding the array size
    if (tokenIndex >= 5) break;
  }
  outputArray[tokenIndex++] = input.substring(startIndex);

  return tokenIndex; 
}

void processCommand(String input) {
  const char delimiter = ' ';
  String tokens[5];

  int tokenCount = splitString(input, delimiter, tokens);

  // commandString in the future will be things such as "doquietmove" or "docastlingmove" or "docapturemove"
  String commandString = tokens[0];

  // moveString should contain a string like "e2e4"
  String moveString = tokens[1];

  if(commandString == "doquietmove") {
    if (moveString.length() > 0) {
        performQuietMove(moveString, stringToPieceType(tokens[2]));
        Serial.println("Executed move: " + moveString);
      } else {
        Serial.println("Error: MOVE command requires an argument.");
      }
  }
  if(commandString == "docapturemove") {
    if (moveString.length() > 0) {
        // the tokens should go as follows, example: e3d4 pawn queen. This means a pawn captured a queen on d4 from d3
        performCaptureMove(moveString, stringToPieceType(tokens[2]), stringToPieceType(tokens[3]));
        Serial.println("Executed move: " + moveString);
      } else {
        Serial.println("Error: MOVE command requires an argument.");
      }
  }
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input.length() > 0) {
      processCommand(input);
    }
  }
}