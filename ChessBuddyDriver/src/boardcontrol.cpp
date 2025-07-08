#include <AccelStepper.h>
#include <string.h>
#include <Main_Definitions.h>
#include <boardcontrol.h>
#include <serverInterface.h>
#include <gui.h>
#include <gui_gateway.h>


// TODO: move these and movehistory variables into a gamestate.cpp/.h file pair
char moveHistory[MAX_MOVES][MOVE_LENGTH];
int moveCount = 0;
bool calibrationStatus = false;
bool activeGame = false;
bool userSideToMove = false;

static std::string startFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

struct Square {
    SquareStatus status; // 0=empty, 1=occupied, 2=potentially_captured
    SquareColor color;  // 0=none, 1=white, 2=black
};

Square squareStates[64];
Square currentBoardState[64];


// for board piece detection
int potentialMovedFromSquare;
int potentialMovedToSquare;
int numPiecesPickedUp;
bool captureMove;

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


// array of key value pairs for each piece offset
// this array is necessary because each physical piece has a different height on the chess board, the robot needs to compensate for each of those appropriately
// KeyValuePair PieceZAxisOffsets[] = {
//     {PieceType::Pawn, -3740},
//     {PieceType::Knight, -2980},
//     {PieceType::Bishop, -2500},
//     {PieceType::Rook, -3370},
//     {PieceType::Queen, -1940},
//     {PieceType::King, -650} // king is good
// };

KeyValuePair PieceZAxisOffsets[] = {
    {PieceType::Pawn, -5240},
    {PieceType::Knight, -4480},
    {PieceType::Bishop, -4000},
    {PieceType::Rook, -4870},
    {PieceType::Queen, -3440},
    {PieceType::King, -2150} // king is good
};



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
          motor.run();    // continue running until switch is triggered
        }

        Serial.println("Axis Calibrated!");
        motor.setCurrentPosition(0); // set zero position after calibration

        // reset speed and accel settings
        setNormalMotorSettings();

        delay(500);
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
StepperMotor xStepperMotor(xStepPin, xDirPin, baseStepperSpeed / 3, baseStepperSpeed, baseStepperSpeed / 10, baseStepperSpeed);
StepperMotor yStepperMotor(yStepPin, yDirPin, baseStepperSpeed, baseStepperSpeed, baseStepperSpeed / 3, baseStepperSpeed);
StepperMotor zStepperMotor(zStepPin, zDirPin, baseStepperSpeed * 4, baseStepperSpeed * baseStepperAccelScalar, baseStepperSpeed / 3, baseStepperSpeed); 

//AccelStepper xStepper(AccelStepper::DRIVER, xStepPin, xDirPin);
//AccelStepper yStepper(AccelStepper::DRIVER, yStepPin, yDirPin);
//AccelStepper zStepper(AccelStepper::DRIVER, zStepPin, zDirPin);

  
// calibrate all X,Y,Z starting positions
void runCalibrationRoutine() {

  // ensure arm clearance for y axis calibration routine
  xStepperMotor.setNormalMotorSettings();
  //zStepperMotor.moveTo(3500);

  // THIS LINE IS TEMPORARY, ITS TO ENSURE THE ARM DOES NOT TURN PAST THE LIMIT SWITCH
  xStepperMotor.moveTo(500);
  //

  xStepperMotor.calibrate(xLimitPin);
  //xStepperMotor.moveTo(1810);

  yStepperMotor.calibrate(yLimitPin);
  yStepperMotor.moveTo(10280);

  zStepperMotor.calibrate(zLimitPin);
  zStepperMotor.moveTo(11000);
  xStepperMotor.moveTo(729);
  
  xStepperMotor.motor.setCurrentPosition(0);
  yStepperMotor.motor.setCurrentPosition(0);
  zStepperMotor.motor.setCurrentPosition(0);

  gotoParkPosition();
  calibrationStatus = true;
}

void gotoParkPosition() {
  // go to park position
  inverseKinematics(-100, 0);
}

int* getSquarePosition(char square[]) {
    // maps the square identifiers to their corresponding array indices
    int index = 0;
    char file = square[0]; // 'a' to 'h'
    char rank = square[1]; // '1' to '8'

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
void performQuietMove(char fromSquare[], char toSquare[], PieceType pieceType = PieceType::King, SpecialMove specialMove = SpecialMove::None) {
  
  int pieceZOffset = getPieceZOffset(pieceType);

  moveToSquare(fromSquare);

  //move down and trigger magnet high
  zStepperMotor.moveTo(pieceZOffset);
  digitalWrite(electromagnetPin, HIGH);

  // TODO: THIS MAY NEED TO BE TWEAKED FURTHER, for example: pawns do not need to be lifted as high to ensure clearance over every other piece
  zStepperMotor.moveTo(8300 + pieceZOffset);

  moveToSquare(toSquare);

  zStepperMotor.moveTo(pieceZOffset);
  digitalWrite(electromagnetPin, LOW);
  zStepperMotor.moveTo(5800);

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
// void performCaptureMove(String moveString, PieceType pieceType = PieceType::King, PieceType capturedPieceType = PieceType::King, SpecialMove specialMove = SpecialMove::None) {
//   size_t length = moveString.length();
//   size_t midpoint = length / 2;

//   int pieceZOffset = getPieceZOffset(pieceType);
//   int capturedPieceZOffset = getPieceZOffset(capturedPieceType);
  
//   // split the move string into two individual squares
//   String fromSquare = moveString.substring(0, midpoint);
//   String toSquare = moveString.substring(midpoint);  

//   // extra steps for capture, we need to visit the "to square" first
//   moveToSquare(toSquare);

//   //move down and trigger magnet high
//   zStepperMotor.moveTo(capturedPieceZOffset);
//   digitalWrite(electromagnetPin, HIGH);
//   zStepperMotor.moveTo(8200 + capturedPieceZOffset);
//   // go to park position to drop off captured piece
//   gotoParkPosition();
  
//   //zStepperMotor.moveTo(5500);
//   digitalWrite(electromagnetPin, LOW);
//   //zStepperMotor.moveTo(0);

//   moveToSquare(fromSquare);

//   //move down and trigger magnet high
//   zStepperMotor.moveTo(pieceZOffset);
//   digitalWrite(electromagnetPin, HIGH);

//   zStepperMotor.moveTo(8000 + pieceZOffset);

//   moveToSquare(toSquare);

//   zStepperMotor.moveTo(pieceZOffset);
//   digitalWrite(electromagnetPin, LOW);
//   zStepperMotor.moveTo(5500);

//   gotoParkPosition();
// }

void moveToSquare(char square[]) {
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

PieceType stringToPieceType(const char* pieceStr) {
  if (strchr(pieceStr, '0') != nullptr) {
    return PieceType::Pawn;
  } else if (strchr(pieceStr, '1') != nullptr) {
    return PieceType::Bishop;
  } else if (strchr(pieceStr, '2') != nullptr) {
    return PieceType::Knight;
  } else if (strchr(pieceStr, '3') != nullptr) {
    return PieceType::Rook;
  } else if (strchr(pieceStr, '4') != nullptr) {
    return PieceType::Queen;
  } else if (strchr(pieceStr, '5') != nullptr) {
    return PieceType::King;
  } else {
    Serial.println("Error: Invalid piece type.");
    return PieceType::King;  // fallback to King
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

    // Compute absolute step magnitudes
  long xAbsSteps = abs(xSteps);
  long yAbsSteps = abs(ySteps);
  long maxSteps = max(xAbsSteps, yAbsSteps);

  if(maxSteps == 0) {
    return;
  }

  // Scale each motor's speed to its share of the move
  float xSpeed = baseStepperSpeed * ((float)xAbsSteps / maxSteps);
  float ySpeed = baseStepperSpeed * ((float)yAbsSteps / maxSteps);
  xStepperMotor.motor.setMaxSpeed(xSpeed);
  yStepperMotor.motor.setMaxSpeed(ySpeed);

  xStepperMotor.motor.setAcceleration(xSpeed * baseStepperAccelScalar);
  yStepperMotor.motor.setAcceleration(ySpeed * baseStepperAccelScalar);

  xStepperMotor.moveToNoRun(xSteps);
  yStepperMotor.moveToNoRun(ySteps);
  
  while (xStepperMotor.motor.distanceToGo() != 0 || 
         yStepperMotor.motor.distanceToGo() != 0) {
    
    xStepperMotor.motor.run();
    yStepperMotor.motor.run();
  }
}

uint8_t splitString(const char* input, char delimiter, char tokens[][28], uint8_t maxTokens = 5) {
  uint8_t tokenIndex = 0;
  uint8_t charIndex = 0;

  while (*input && tokenIndex < maxTokens) {
    if (*input == delimiter) {
      tokens[tokenIndex][charIndex] = '\0';
      tokenIndex++;
      charIndex = 0;
    } else if (charIndex < 27) {
      tokens[tokenIndex][charIndex++] = *input;
    }
    input++;
  }

  tokens[tokenIndex][charIndex] = '\0'; 
  return tokenIndex + 1;
}


/* this function edits the square state board to reflect the engine move */
// TODO: THIS WILL NEED EDITS FOR DIFFERENT MOVE TYPES LIKE CASTLING, EN PASSANT, AND PROMOTIONS
// should be totally functional for quiet moves and capture moves
void editSquareStates(uint8_t fromSquare, uint8_t toSquare) {
  if (fromSquare < 0 || fromSquare > 63 || toSquare < 0 || toSquare > 63) {
    Serial.println("Error: Square index out of bounds");
    return;
  }
  Square temp = squareStates[fromSquare];
  squareStates[fromSquare].status = SquareStatus::Empty;
  squareStates[fromSquare].color = SquareColor::None;
  squareStates[toSquare] = temp;
}

void algebraicToSquares(const char move[], uint8_t& fromSquare, uint8_t& toSquare) {
  char fromFile = move[0];
  char fromRank = move[1];
  char toFile = move[2];
  char toRank = move[3];

  fromSquare = ((fromRank - '1') * 8) + (fromFile - 'a');
  toSquare = ((toRank - '1') * 8) + (toFile - 'a');
}

// void processCommand(String input) {
//   const char delimiter = ' ';
//   String tokens[5];

//   int tokenCount = splitString(input, delimiter, tokens);

//   // commandString in the future will be things such as "doquietmove" or "docastlingmove" or "docapturemove"
//   String commandString = tokens[0];

//   // moveString should contain a string like "e2e4"
//   String moveString = tokens[1];

//   int fromSquare = 0, toSquare = 0;
//   algebraicToSquares(moveString, fromSquare, toSquare);

//   if(commandString == "startNewGame") {
//     boardStartNewGame();
//   }

//   // usage: moveToSquare <square> <piecetype>
//   if(commandString == "moveToSquare") {
//     if (moveString.length() > 0) {
//         PieceType pieceType = stringToPieceType(tokens[2]);
//         int pieceZOffset = getPieceZOffset(pieceType);
//         moveToSquare(moveString);

//          //move down and trigger magnet high
//         zStepperMotor.moveTo(pieceZOffset);
//         digitalWrite(electromagnetPin, HIGH);
      
//         // TODO: THIS "6000" NEEDS TO BE TWEAKED, for example: pawns do not need to be lifted as high to ensure clearance over every other piece
//         zStepperMotor.moveTo(7000 + pieceZOffset);
//         Serial.println("Moved to square: " + moveString);
//       } else {
//         Serial.println("Error: MOVE command requires an argument.");
//       }
//   }

//   if(commandString == "doquietmove") {
//     if (moveString.length() > 0) {
//         performQuietMove(moveString, stringToPieceType(tokens[2]));
//         Serial.println("Executed move: " + moveString);
//       } else {
//         Serial.println("Error: MOVE command requires an argument.");
//       }
//     // switch back to user's move
//     editSquareStates(fromSquare, toSquare);
//     deduceUserMove();
//   }
//   if(commandString == "docapturemove") {
//     if (moveString.length() > 0) {
//         // the tokens should go as follows, example: e3d4 pawn queen. This means a pawn captured a queen on d4 from d3
//         performCaptureMove(moveString, stringToPieceType(tokens[2]), stringToPieceType(tokens[3]));
//         Serial.println("Executed move: " + moveString);
//       } else {
//         //Serial.println("Error: MOVE command requires an argument.");
//       }

//     // switch back to user's move
//     editSquareStates(fromSquare, toSquare);
//     deduceUserMove();
//   }

//   if(commandString == "dokingsidecastle") {
//     performKingSideCastle(moveString);
//     editSquareStates(61, 63);
//     editSquareStates(64, 62);
//     deduceUserMove();
//   }

//   if(commandString == "doqueensidecastle") {
//     performQueenSideCastle(moveString);
//     editSquareStates(61, 59);
//     editSquareStates(57, 60);
//     deduceUserMove();
//   }
// }

// quick helper function to add a move to the movecount
void addMove(const char* move) {
    if (moveCount < MAX_MOVES) {
        strncpy(moveHistory[moveCount], move, MOVE_LENGTH - 1);
        moveHistory[moveCount][MOVE_LENGTH - 1] = '\0';
        moveCount++;
    }
}

void clearMoveHistory() {
    for (int i = 0; i < moveCount; i++) {
        moveHistory[i][0] = '\0';  // Clear each move string
    }
    moveCount = 0;  // Reset the move counter
}

// void handleArmMove(const char* move) {
//   const char delimiter = '|';
//   char tokens[5][10];

//   uint8_t tokenCount = splitString(move, delimiter, tokens);

//   // moveString should contain a string like "e2e4"
//   char moveString[5];

//   strcpy(moveString, tokens[0]);

//   char moveType[14];
//   strcpy(moveString, tokens[1]);

//   // adds arm move to the movelist
//   addMove(moveString);

//   uint8_t fromSquare = 0, toSquare = 0;
//   algebraicToSquares(moveString, fromSquare, toSquare);

//   // if the move string returned comes in the format "bestmove b1c3|movedPiece", must be quiet move
//   if(tokenCount <= 2) {
//     performQuietMove(moveString, stringToPieceType(moveType));
//   }

//   editSquareStates(fromSquare, toSquare);


//   end_engine_turn_handler();
// }

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

void printMoveHistory() {
  Serial.println("Move History:");
  for (int i = 0; i < moveCount; i++) {
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println(moveHistory[i]);
  }
}


void handleArmMove(const char* move) {
  const char delimiter = '|';
  char tokens[5][28];

  uint8_t tokenCount = splitString(move, delimiter, tokens);

  // moveString (first tokens slot) should contain a string like "e2e4"
  char moveString[6];
  strcpy(moveString, tokens[0]);

  uint8_t fromSquareIndex = 0, toSquareIndex = 0;
  algebraicToSquares(moveString, fromSquareIndex, toSquareIndex);

  size_t length = strlen(moveString);
  size_t midpoint = length / 2;

  // split the move string into two individual squares
  char fromSquare[3];
  char toSquare[3];
  char promotionChar = NULL;

  strncpy(fromSquare, moveString, midpoint);
  fromSquare[2] = '\0';

  char* midpointPtr = &(moveString[midpoint]);
  strncpy(toSquare, midpointPtr, 2);
  toSquare[2] = '\0';


  // adds arm move to the movelist
  // if the move string returned comes in the format "bestmove b1c3|movedPiece", must be quiet move
  if(tokenCount <= 2) {
    addMove(moveString);
    performQuietMove(fromSquare, toSquare, stringToPieceType(tokens[1]));
  } else {
    int8_t specialMoveTokenIndex = -1;
    int8_t capturedPieceTokenIndex = -1;
    int8_t promotionTokenIndex = -1;

    for(int i = 0; i < 5; i++) {
      if(strstr(tokens[i], "capturedPiece") != nullptr) {
        capturedPieceTokenIndex = i;
      }
      if(strstr(tokens[i], "specialMove") != nullptr) {
        specialMoveTokenIndex = i;
      }
      if(strstr(tokens[i], "promotion") != nullptr) {
        promotionTokenIndex = i;
      }
    }

    // handle these other move scenarios
    if(capturedPieceTokenIndex != -1) {
      // perform capture move
    }

    if(specialMoveTokenIndex != -1) {
      // perform special moves
    }

    if(promotionTokenIndex != -1) {
      // do promotions
    }
  }

  editSquareStates(fromSquareIndex, toSquareIndex);

  printMoveHistory();

  // swap back to user turn to move   
  resetPieceDetectionParameters();
  userSideToMove = true;

  request_end_engine_turn();
}

void instantiateBoardState() {
  // for all the white pieces
  for (int i = 0; i < 16; i++) {
    squareStates[i].status = SquareStatus::Occupied;
    squareStates[i].color = SquareColor::White;
  }

  // for all empty middle squares from starting position
  for(int i = 16; i < 48; i++) {
    squareStates[i].status = SquareStatus::Empty;
    squareStates[i].color = SquareColor::None;
  }
  
  // for all the black pieces
  for (int i = 48; i < 64; i++) {
    squareStates[i].status = SquareStatus::Occupied;
    squareStates[i].color = SquareColor::Black;
  }
}

void updateCurrentBoardState() {
  for (int i = 0; i < 64; i++) {
    currentBoardState[i] = squareStates[i];
  }
}

void resetPieceDetectionParameters() {
  potentialMovedFromSquare = -1;
  numPiecesPickedUp = 0;
  potentialMovedToSquare = -1;
  captureMove = false;
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



void scanningUserMove(bool isUserSideToMove = false, bool isFinalizedMove = false) {
  // do not poll board if a game is not active
  if(!activeGame) {
    return;
  }

  // do not poll board if it is not the user's turn
  if(!isUserSideToMove) {
    return;
  }

  //updateCurrentBoardState();

  uint64_t binaryBoardState = readShiftRegisters();

  potentialMovedFromSquare = -1;
  numPiecesPickedUp = 0;
  if (!captureMove) potentialMovedToSquare = -1;


  // Update board state
  for (int i = 0; i < 64; i++) {
    currentBoardState[i].status = (binaryBoardState >> i) & 1 ? SquareStatus::Occupied : SquareStatus::Empty;
  }
  
  Serial.print("square states: ");
  for (int i = numBits - 1; i >= 0; i--) {                                                  
    Serial.print(squareStates[i].status); // Print each bit
    if (i % 8 == 0) Serial.print(" ");  // Add space after every 8 bits
  }
  Serial.println();


  Serial.print("current board state: ");
  for (int i = numBits - 1; i >= 0; i--) {                                                  
    Serial.print(currentBoardState[i].status); // Print each bit
    if (i % 8 == 0) Serial.print(" ");  // Add space after every 8 bits
  }
  Serial.println();

  // Detect move
  int friendlyPieceCount = 0;
  for (int i = 0; i < 64; i++) {
    if (currentBoardState[i].status != squareStates[i].status && squareStates[i].status == SquareStatus::Occupied) {
      if (squareStates[i].color == SquareColor::White) {
        if(potentialMovedFromSquare == -1) {
          potentialMovedFromSquare = i + 1;
          Serial.print("Potential moved from square: ");
          Serial.println(potentialMovedFromSquare);
        }
        friendlyPieceCount++;
      }
    }
  }
  numPiecesPickedUp = friendlyPieceCount;

  for(int i = 0; i < 64; i++) {
    if (currentBoardState[i].status != squareStates[i].status && squareStates[i].status == 1) {
      if (currentBoardState[i].color == 2) {
        potentialMovedToSquare = i + 1;
        captureMove = true;
        Serial.print("Potential moved to square (capture): ");
        Serial.println(potentialMovedToSquare);
      }
    }
  }

  
  for(int i = 0; i < 64; i++) {
    if (currentBoardState[i].status == SquareStatus::Occupied && squareStates[i].status == SquareStatus::Empty && !captureMove) {
      potentialMovedToSquare = i + 1;
      Serial.print("Potential moved to square (quiet move): ");
      Serial.println(potentialMovedToSquare);
    }
  }


  // TODO: DELETE THIS SOON
  // if(isFinalizedMove) {

  // } else if(potentialMovedFromSquare == -1 && captureMove) {

  // }

  if(potentialMovedFromSquare == -1 && captureMove && !isFinalizedMove) {
    potentialMovedToSquare = -1;
    captureMove = false;
  }


  Serial.print("Potential moved from square: ");
  Serial.println(potentialMovedFromSquare);

  if(!captureMove) {
    Serial.print("Potential moved to square (quiet move): ");
  } else {
      Serial.print("Potential moved to square (capture move): ");
  }
  Serial.println(potentialMovedToSquare);

  // Check move validity
  if (potentialMovedFromSquare == -1 || potentialMovedToSquare == -1) {
    Serial.println("ERROR - Retry");
    return;  // Retry instead of recurse
  }

  String finalizedMove;

  // DEBUGGING INFO TODO: REFACTOR LATER
  if (numPiecesPickedUp >= 2) {
    if (potentialMovedToSquare == 6 || potentialMovedToSquare == 7) {
      // editSquareStates(4, 6);
      // editSquareStates(7, 5);
      finalizedMove = combineSquareStrings(5, 7);
      Serial.println(finalizedMove);
    } else {
      // editSquareStates(4, 2);
      // editSquareStates(0, 3);
      finalizedMove = combineSquareStrings(5, 3);
      Serial.println(finalizedMove);
    }
  } else {
    //editSquareStates(potentialMovedFromSquare - 1, potentialMovedToSquare - 1);
    finalizedMove = combineSquareStrings(potentialMovedFromSquare, potentialMovedToSquare);
    Serial.println(finalizedMove);
  }

  // once the move is finalized, edit square states (the user ended their turn) 
  if(isFinalizedMove) {

    // Process move (castling or normal)
    if (numPiecesPickedUp >= 2) {
      if (potentialMovedToSquare == 6 || potentialMovedToSquare == 7) {
        editSquareStates(4, 6);
        editSquareStates(7, 5);
        finalizedMove = combineSquareStrings(5, 7);
        Serial.println(finalizedMove);
      } else {
        editSquareStates(4, 2);
        editSquareStates(0, 3);
        finalizedMove = combineSquareStrings(5, 3);
        Serial.println(finalizedMove);
      }
    } else {
      editSquareStates(potentialMovedFromSquare - 1, potentialMovedToSquare - 1);
      finalizedMove = combineSquareStrings(potentialMovedFromSquare, potentialMovedToSquare);
      Serial.println(finalizedMove);
    }

    addMove(finalizedMove.c_str());
  }


  // addMove(finalizedMove.c_str());
}

// void deduceUserMove() {
//   bool validMove = false;
//   userSideToMove = true;
  
//   // Wait for button to be released (assuming LOW is pressed)
//   // while (digitalRead(buttonPin) == LOW) {
//   //     delay(10); // Small delay to avoid busy-waiting
//   // }
    
//   while (!validMove) {
//     // Copy initial state
//     for (int i = 0; i < 64; i++) {
//       currentBoardState[i] = squareStates[i];
//     }
//     int potentialMovedFromSquare = -1;
//     int potentialMovedToSquare = -1;
//     int numPiecesPickedUp = 0;
//     bool captureMove = false;

//     while (userSideToMove) {
//       //digitalWrite(ledPin, HIGH); // Turn LED on
//       uint64_t binaryBoardState = readShiftRegisters();
//       delay(50);
//       uint64_t stableBoardState = readShiftRegisters();
//       if (binaryBoardState != stableBoardState) continue;

//       potentialMovedFromSquare = -1;
//       numPiecesPickedUp = 0;
//       if (!captureMove) potentialMovedToSquare = -1;

//       // Update board state
//       for (int i = 0; i < 64; i++) {
//         currentBoardState[i].status = (binaryBoardState >> i) & 1 ? SquareStatus::Occupied : SquareStatus::Empty;
//       }

//       // Detect move
//       for (int i = 0; i < 64; i++) {
//         if (currentBoardState[i].status != squareStates[i].status && squareStates[i].status == 1) {
//           if (currentBoardState[i].color == 1) {
//             if (potentialMovedFromSquare == -1) {
//               potentialMovedFromSquare = i + 1;
//               Serial.print("Potential moved from square: ");
//               Serial.println(potentialMovedFromSquare);
//             }
//             numPiecesPickedUp++;
//           }
//           if (currentBoardState[i].color == 2) {
//             potentialMovedToSquare = i + 1;
//             captureMove = true;
//             Serial.print("Potential moved to square (capture): ");
//             Serial.println(potentialMovedToSquare);
//           }
//         }
//         if (currentBoardState[i].status == 1 && squareStates[i].status == 0 && !captureMove) {
//           if (potentialMovedToSquare == -1) {
//             potentialMovedToSquare = i + 1;
//             Serial.print("Potential moved to square (quiet move): ");
//             Serial.println(potentialMovedToSquare);
//           }
//         }
//       }
//       delay(50);
//     }
//     //digitalWrite(ledPin, LOW); // Turn LED off

//     // Check validity
//     if (potentialMovedFromSquare == -1 || potentialMovedToSquare == -1) {
//       Serial.println("ERROR - Retry");
//       delay(1000);
//       continue;  // Retry instead of recurse
//     }
//     validMove = true;
//     String finalizedMove;

//     // Process move (castling or normal)
//     if (numPiecesPickedUp >= 2) {
//       if (potentialMovedToSquare == 6 || potentialMovedToSquare == 7) {
//         editSquareStates(4, 6);
//         editSquareStates(7, 5);
//         finalizedMove = combineSquareStrings(5, 7);
//         Serial.println(finalizedMove);
//       } else {
//         editSquareStates(4, 2);
//         editSquareStates(0, 3);
//         finalizedMove = combineSquareStrings(5, 3);
//         Serial.println(finalizedMove);
//       }
//     } else {
//       editSquareStates(potentialMovedFromSquare - 1, potentialMovedToSquare - 1);
//       finalizedMove = combineSquareStrings(potentialMovedFromSquare, potentialMovedToSquare);
//       Serial.println(finalizedMove);
//     }

//     // addMove(finalizedMove.c_str());
//     // moveCount++;
//   }
// }

// starts a new chess game
void boardStartNewGame() {
  //gotoParkPosition();

  clearMoveHistory();
  moveCount = 0;
  activeGame = true;
  // TODO: depending on if user is playing white or black, this might need to change
  userSideToMove = true;
  instantiateBoardState();
  updateCurrentBoardState();
  resetPieceDetectionParameters();
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
  pinMode(xLimitPin, INPUT); 
  pinMode(yLimitPin, INPUT); 
  pinMode(zLimitPin, INPUT);
  
  // button pin for user move completion feedback
  //pinMode(buttonPin, INPUT_PULLUP);

  // Set built-in LED pin as output
  //pinMode(ledPin, OUTPUT);

  // specifies board electronics pin configurations
  pinMode(latchPin, OUTPUT);
  pinMode(clockEnablePin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataIn, INPUT);
  
  digitalWrite(clockPin, LOW); // ensure clock starts low
  
  digitalWrite(clockEnablePin, LOW); // ensure clock is enabled
  digitalWrite(latchPin, HIGH);
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
