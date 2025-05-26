#include <AccelStepper.h>
#include <string.h>
#include <Main_Definitions.h>
#include <boardcontrol.h>

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
        
        motor.moveTo(-30000);  // move far in reverse
  
        while (digitalRead(limitSwitchPin) == LOW) {
          Serial.println("Axis Calibrated");
          motor.run();    // continue running until switch is triggered
        }

        motor.setCurrentPosition(0); // set zero position after calibration

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
StepperMotor xStepperMotor(xStepPin, xDirPin, baseStepperSpeed, 1000, baseStepperSpeed / 8, 1000);
StepperMotor yStepperMotor(yStepPin, yDirPin, baseStepperSpeed * 4, 2500, 1000, 1000);
StepperMotor zStepperMotor(zStepPin, zDirPin, baseStepperSpeed * 5, 10000, baseStepperSpeed, 1000); 

//AccelStepper xStepper(AccelStepper::DRIVER, xStepPin, xDirPin);
//AccelStepper yStepper(AccelStepper::DRIVER, yStepPin, yDirPin);
//AccelStepper zStepper(AccelStepper::DRIVER, zStepPin, zDirPin);

void runCalibrationRoutine() {

  // ensure arm clearance for y axis calibration routine
  zStepperMotor.setNormalMotorSettings();
  //zStepperMotor.moveTo(3500);

  // THIS LINE IS TEMPORARY, ITS TO ENSURE THE ARM DOES NOT TURN PAST THE LIMIT SWITCH
  xStepperMotor.moveTo(500);
  //

  xStepperMotor.calibrate(xLimitPin);
  //xStepperMotor.moveTo(1810);

  yStepperMotor.calibrate(yLimitPin);
  yStepperMotor.moveTo(10280);

  zStepperMotor.calibrate(zLimitPin);
  zStepperMotor.moveTo(9500);
  xStepperMotor.moveTo(729);
}

void gotoParkPosition() {
  // go to park position
  inverseKinematics(-100, 0);
}

int* getSquarePosition(const String& square) {
    // maps the square identifiers to their corresponding array indices
    int index = 0;
    char file = square.charAt(0); // 'a' to 'h'
    char rank = square.charAt(1); // '1' to '8'

    // NOTE: this is for the robot playing from black's perspective
    // flip the file so 'a' -> 7, 'b' -> 6, ..., 'h' -> 0
    int fileIndex = 7 - (file - 'a');
    
    // reverse the rank for black's perspective: rank 1 -> 7, rank 8 -> 0
    int rankIndex = 7 - (rank - '1');
    
    index = rankIndex * 8 + fileIndex;
    
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
  zStepperMotor.moveTo(8200 + pieceZOffset);

  moveToSquare(toSquare);

  zStepperMotor.moveTo(pieceZOffset);
  digitalWrite(electromagnetPin, LOW);
  zStepperMotor.moveTo(5500);

  gotoParkPosition();
}

void performKingSideCastle(String moveString) {
  
  int kingZOffset = getPieceZOffset(PieceType::King);
  int rookZOffset = getPieceZOffset(PieceType::Rook);

  moveToSquare("e8");
  
  //move down and trigger magnet high
  zStepperMotor.moveTo(kingZOffset);
  digitalWrite(electromagnetPin, HIGH);
  
  zStepperMotor.moveTo(8200 + kingZOffset);

  moveToSquare("g8");

  zStepperMotor.moveTo(kingZOffset);
  digitalWrite(electromagnetPin, LOW);
  zStepperMotor.moveTo(5500);
  
  // move the rook
  moveToSquare("h8");
  
  //move down and trigger magnet high
  zStepperMotor.moveTo(rookZOffset);
  digitalWrite(electromagnetPin, HIGH);
  
  zStepperMotor.moveTo(8200 + rookZOffset);

  moveToSquare("f8");
  
  zStepperMotor.moveTo(rookZOffset);
  digitalWrite(electromagnetPin, LOW);
  zStepperMotor.moveTo(5500);

  gotoParkPosition();
  
}

void performQueenSideCastle(String moveString) {
  int kingZOffset = getPieceZOffset(PieceType::King);
  int rookZOffset = getPieceZOffset(PieceType::Rook);

  moveToSquare("e8");
  
  //move down and trigger magnet high
  zStepperMotor.moveTo(kingZOffset);
  digitalWrite(electromagnetPin, HIGH);
  
  zStepperMotor.moveTo(8200 + kingZOffset);

  moveToSquare("c8");

  zStepperMotor.moveTo(kingZOffset);
  digitalWrite(electromagnetPin, LOW);
  zStepperMotor.moveTo(5500);
  
  // move the rook
  moveToSquare("a8");
  
  //move down and trigger magnet high
  zStepperMotor.moveTo(rookZOffset);
  digitalWrite(electromagnetPin, HIGH);
  
  zStepperMotor.moveTo(8200 + rookZOffset);

  moveToSquare("d8");
  
  zStepperMotor.moveTo(rookZOffset);
  digitalWrite(electromagnetPin, LOW);
  zStepperMotor.moveTo(5500);

  gotoParkPosition();
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
  zStepperMotor.moveTo(8200 + capturedPieceZOffset);
  // go to park position to drop off captured piece
  gotoParkPosition();
  
  //zStepperMotor.moveTo(5500);
  digitalWrite(electromagnetPin, LOW);
  //zStepperMotor.moveTo(0);

  moveToSquare(fromSquare);

  //move down and trigger magnet high
  zStepperMotor.moveTo(pieceZOffset);
  digitalWrite(electromagnetPin, HIGH);

  zStepperMotor.moveTo(8000 + pieceZOffset);

  moveToSquare(toSquare);

  zStepperMotor.moveTo(pieceZOffset);
  digitalWrite(electromagnetPin, LOW);
  zStepperMotor.moveTo(5500);

  gotoParkPosition();
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

/* this function edits the square state board to reflect the engine move */
// TODO: THIS WILL NEED EDITS FOR DIFFERENT MOVE TYPES LIKE CASTLING, EN PASSANT, AND PROMOTIONS
// should be totally functional for quiet moves and capture moves
void editSquareStates(int fromSquare, int toSquare) {
  if (fromSquare < 0 || fromSquare > 63 || toSquare < 0 || toSquare > 63) {
    Serial.println("Error: Square index out of bounds");
    return;
  }
  Square temp = squareStates[fromSquare];
  squareStates[fromSquare].status = 0;
  squareStates[fromSquare].color = 0;
  squareStates[toSquare] = temp;
}

void algebraicToSquares(const String& move, int& fromSquare, int& toSquare) {
  char fromFile = move[0];
  char fromRank = move[1];
  char toFile = move[2];
  char toRank = move[3];

  fromSquare = ((fromRank - '1') * 8) + (fromFile - 'a');
  toSquare = ((toRank - '1') * 8) + (toFile - 'a');
}

void processCommand(String input) {
  const char delimiter = ' ';
  String tokens[5];

  int tokenCount = splitString(input, delimiter, tokens);

  // commandString in the future will be things such as "doquietmove" or "docastlingmove" or "docapturemove"
  String commandString = tokens[0];

  // moveString should contain a string like "e2e4"
  String moveString = tokens[1];

  int fromSquare = 0, toSquare = 0;
  algebraicToSquares(moveString, fromSquare, toSquare);

  if(commandString == "startNewGame") {
    startNewGame();
  }

  // usage: moveToSquare <square> <piecetype>
  if(commandString == "moveToSquare") {
    if (moveString.length() > 0) {
        PieceType pieceType = stringToPieceType(tokens[2]);
        int pieceZOffset = getPieceZOffset(pieceType);
        moveToSquare(moveString);

         //move down and trigger magnet high
        zStepperMotor.moveTo(pieceZOffset);
        digitalWrite(electromagnetPin, HIGH);
      
        // TODO: THIS "6000" NEEDS TO BE TWEAKED, for example: pawns do not need to be lifted as high to ensure clearance over every other piece
        zStepperMotor.moveTo(7000 + pieceZOffset);
        Serial.println("Moved to square: " + moveString);
      } else {
        Serial.println("Error: MOVE command requires an argument.");
      }
  }

  if(commandString == "doquietmove") {
    if (moveString.length() > 0) {
        performQuietMove(moveString, stringToPieceType(tokens[2]));
        Serial.println("Executed move: " + moveString);
      } else {
        Serial.println("Error: MOVE command requires an argument.");
      }
    // switch back to user's move
    editSquareStates(fromSquare, toSquare);
    deduceUserMove();
  }
  if(commandString == "docapturemove") {
    if (moveString.length() > 0) {
        // the tokens should go as follows, example: e3d4 pawn queen. This means a pawn captured a queen on d4 from d3
        performCaptureMove(moveString, stringToPieceType(tokens[2]), stringToPieceType(tokens[3]));
        Serial.println("Executed move: " + moveString);
      } else {
        //Serial.println("Error: MOVE command requires an argument.");
      }

    // switch back to user's move
    editSquareStates(fromSquare, toSquare);
    deduceUserMove();
  }

  if(commandString == "dokingsidecastle") {
    performKingSideCastle(moveString);
    editSquareStates(61, 63);
    editSquareStates(64, 62);
    deduceUserMove();
  }

  if(commandString == "doqueensidecastle") {
    performQueenSideCastle(moveString);
    editSquareStates(61, 59);
    editSquareStates(57, 60);
    deduceUserMove();
  }
  
}

void instantiateBoardState() {
  // for all the white pieces
  for (int i = 0; i < 16; i++) {
    squareStates[i].status = 1;
    squareStates[i].color = 1;
  }

  // for all empty middle squares from starting position
  for(int i = 16; i < 48; i++) {
    squareStates[i].status = 0;
    squareStates[i].color = 0;
  }
  
  // for all the black pieces
  for (int i = 48; i < 64; i++) {
    squareStates[i].status = 1;
    squareStates[i].color = 2;
  }
}

uint64_t readShiftRegisters() {
  // Trigger the latch to store the current state of the inputs
  digitalWrite(latchPin, LOW);
  delayMicroseconds(5); // Small delay for stability
  digitalWrite(latchPin, HIGH);
  delayMicroseconds(5);

  uint64_t result = 0; // Initialize result
  for (int i = 0; i < numBits; i++) {
    digitalWrite(clockPin, LOW);  // Start with clock LOW
    delayMicroseconds(5);         // Small delay for stability
    result <<= 1;                 // Shift result to the left
    result |= digitalRead(dataIn); // Read data bit
    digitalWrite(clockPin, HIGH); // Pulse clock HIGH
    delayMicroseconds(5);         // Small delay
  }
  digitalWrite(clockPin, LOW);    // Ensure clock ends LOW
  return result;
}

String squareNumToAlgebraic(int square) {
  char file = 'a' + (square - 1) % 8;
  char rank = '1' + (square - 1) / 8;
  return String(file) + String(rank);
}

String combineSquareStrings(int fromSquare, int toSquare) {
  return squareNumToAlgebraic(fromSquare) + squareNumToAlgebraic(toSquare);
}


void deduceUserMove() {
  bool validMove = false;
  
  // Wait for button to be released (assuming LOW is pressed)
  while (digitalRead(buttonPin) == LOW) {
      delay(10); // Small delay to avoid busy-waiting
  }
    
  while (!validMove) {
    // Copy initial state
    for (int i = 0; i < 64; i++) {
      currentBoardState[i] = squareStates[i];
    }
    int potentialMovedFromSquare = -1;
    int potentialMovedToSquare = -1;
    int numPiecesPickedUp = 0;
    bool captureMove = false;

    while (digitalRead(buttonPin) == HIGH) {
      digitalWrite(ledPin, HIGH); // Turn LED on
      uint64_t binaryBoardState = readShiftRegisters();
      delay(50);
      uint64_t stableBoardState = readShiftRegisters();
      if (binaryBoardState != stableBoardState) continue;

      potentialMovedFromSquare = -1;
      numPiecesPickedUp = 0;
      if (!captureMove) potentialMovedToSquare = -1;

      // Update board state
      for (int i = 0; i < 64; i++) {
        currentBoardState[i].status = (binaryBoardState >> i) & 1 ? 1 : 0;
      }

      // Detect move
      for (int i = 0; i < 64; i++) {
        if (currentBoardState[i].status != squareStates[i].status && squareStates[i].status == 1) {
          if (currentBoardState[i].color == 1) {
            if (potentialMovedFromSquare == -1) {
              potentialMovedFromSquare = i + 1;
              Serial.print("Potential moved from square: ");
              Serial.println(potentialMovedFromSquare);
            }
            numPiecesPickedUp++;
          }
          if (currentBoardState[i].color == 2) {
            potentialMovedToSquare = i + 1;
            captureMove = true;
            Serial.print("Potential moved to square (capture): ");
            Serial.println(potentialMovedToSquare);
          }
        }
        if (currentBoardState[i].status == 1 && squareStates[i].status == 0 && !captureMove) {
          if (potentialMovedToSquare == -1) {
            potentialMovedToSquare = i + 1;
            Serial.print("Potential moved to square (quiet move): ");
            Serial.println(potentialMovedToSquare);
          }
        }
      }
      delay(50);
    }
    digitalWrite(ledPin, LOW); // Turn LED off

    // Check validity
    if (potentialMovedFromSquare == -1 || potentialMovedToSquare == -1) {
      Serial.println("ERROR - Retry");
      delay(1000);
      continue;  // Retry instead of recurse
    }
    validMove = true;

    // Process move (castling or normal)
    if (numPiecesPickedUp >= 2) {
      if (potentialMovedToSquare == 6 || potentialMovedToSquare == 7) {
        editSquareStates(4, 6);
        editSquareStates(7, 5);
        String finalizedMove = combineSquareStrings(5, 7);
        Serial.println(finalizedMove);
      } else {
        editSquareStates(4, 2);
        editSquareStates(0, 3);
        String finalizedMove = combineSquareStrings(5, 3);
        Serial.println(finalizedMove);
      }
    } else {
      editSquareStates(potentialMovedFromSquare - 1, potentialMovedToSquare - 1);
      String finalizedMove = combineSquareStrings(potentialMovedFromSquare, potentialMovedToSquare);
      Serial.println(finalizedMove);
    }
  }
}

// starts a new chess game
void startNewGame() {
  gotoParkPosition();
  
  instantiateBoardState();
  //whiteToMove = true;
  
  deduceUserMove();
}

void setupBoard() {
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
  
  // button pin for user move completion feedback
  pinMode(buttonPin, INPUT_PULLUP);

  // Set built-in LED pin as output
  pinMode(ledPin, OUTPUT);

  // specifies board electronics pin configurations
  pinMode(latchPin, OUTPUT);
  pinMode(clockEnablePin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataIn, INPUT);
  
  digitalWrite(clockPin, LOW); // ensure clock starts low
  
  digitalWrite(clockEnablePin, LOW); // ensure clock is enabled
  digitalWrite(latchPin, HIGH);
  
  // calibrate all X,Y,Z starting positions
  runCalibrationRoutine();

  xStepperMotor.setNormalMotorSettings();
  yStepperMotor.setNormalMotorSettings();
  
  xStepperMotor.motor.setCurrentPosition(0);
  yStepperMotor.motor.setCurrentPosition(0);
  zStepperMotor.motor.setCurrentPosition(0);
}


// void loop() {
//   if (Serial.available() > 0) {
//     String input = Serial.readStringUntil('\n');
//     input.trim();

//     if (input.length() > 0) {
//       processCommand(input);
//     }
//   }
// }
