#include <AccelStepper.h>
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
  {-105, 124}, 
  {-52, 123},
  // {x, y} coordinates for other squares
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
The X-Axis is referring to the base joint rotation. The Y-Axis is referring to the arm segment joint rotation.
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
StepperMotor xStepperMotor(xStepPin, xDirPin, baseStepperSpeed, baseStepperAccel, baseStepperSpeed / 10, baseStepperAccel / 2);
StepperMotor yStepperMotor(yStepPin, yDirPin, baseStepperSpeed * 20, baseStepperAccel * 4, 1250.0 , baseStepperAccel);
StepperMotor zStepperMotor(zStepPin, zDirPin, baseStepperSpeed, baseStepperAccel, baseStepperSpeed / 4, baseStepperAccel / 2.5); 

AccelStepper xStepper(AccelStepper::DRIVER, xStepPin, xDirPin);
AccelStepper yStepper(AccelStepper::DRIVER, yStepPin, yDirPin);
AccelStepper zStepper(AccelStepper::DRIVER, zStepPin, zDirPin);

void setup() {
  Serial.begin(9600);

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

  //movePiece();

  //inverseKinematics(SquarePositions[0][0], SquarePositions[0][1]);
  //delay(2000);
  // inverseKinematics(SquarePositions[1][0], SquarePositions[1][1]);
  // delay(2000);
  // inverseKinematics(SquarePositions[0][0], SquarePositions[0][1]);
  // delay(2000);
  // inverseKinematics(SquarePositions[1][0], SquarePositions[1][1]);
  inverseKinematics(4,115);
}

void runCalibrationRoutine() {

   // ensure arm clearance for y axis calibration routine
  //zStepperMotor.setNormalMotorSettings();
  //zStepperMotor.moveTo(3000);

  xStepperMotor.calibrate(xLimitPin);
  xStepperMotor.moveTo(560);

  yStepperMotor.calibrate(yLimitPin);
  yStepperMotor.moveTo(11400);

  //zStepperMotor.calibrate(zLimitPin);
  //zStepperMotor.moveTo(8500);
  xStepperMotor.moveTo(1550);
  
}

void movePiece() {

  xStepperMotor.moveToNoRun(xStepperMotor.motor.currentPosition() - 400);
  yStepperMotor.moveToNoRun(yStepperMotor.motor.currentPosition() + 7000);
  while (xStepperMotor.motor.distanceToGo() != 0 || 
         yStepperMotor.motor.distanceToGo() != 0) {
    
    xStepperMotor.motor.run();
    yStepperMotor.motor.run();
  }

  zStepperMotor.moveTo(zStepperMotor.motor.currentPosition() - 1000);
  digitalWrite(electromagnetPin, HIGH);
  zStepperMotor.moveTo(zStepperMotor.motor.currentPosition() + 1000);
  
  xStepperMotor.moveToNoRun(xStepperMotor.motor.currentPosition() + 400);
  yStepperMotor.moveToNoRun(yStepperMotor.motor.currentPosition() - 2000);
  while (xStepperMotor.motor.distanceToGo() != 0 || 
         yStepperMotor.motor.distanceToGo() != 0) {
    
    xStepperMotor.motor.run();
    yStepperMotor.motor.run();
  }
  zStepperMotor.moveTo(zStepperMotor.motor.currentPosition() - 1000);
  digitalWrite(electromagnetPin, LOW);
  zStepperMotor.moveTo(zStepperMotor.motor.currentPosition() + 1000);
  
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
  q1 = q1 * (180 / M_PI);
  q2 = q2 * (180 / M_PI);

  long targetYSteps = floor((q2 * stepsPerRevolution * ArmGearReductionRatio) / 360);
  long targetXSteps = floor((q1 * stepsPerRevolution * BaseGearReductionRatio) / 360);


  Serial.print("targetXSteps: ");
  Serial.println(targetXSteps);

  Serial.print("targetYSteps: ");
  Serial.println(targetYSteps);

  long yStepperPos = yStepperMotor.motor.currentPosition();
  long xStepperPos = xStepperMotor.motor.currentPosition();

  long xSteps = targetXSteps - xStepperPos;
  long ySteps = targetYSteps - abs(yStepperPos); 

  if(targetXSteps < abs(xStepperPos)) {
    xSteps = targetXSteps + xStepperPos;
  } else {
    xSteps = targetXSteps + xStepperPos;
  }


  Serial.print("x stepper current position: ");
  Serial.println(xStepperPos);

  Serial.print("y stepper current position: ");
  Serial.println(yStepperPos);

  // If moving left (negative x), make steps negative
  // if (x < 0) {
  //     xSteps = -xSteps;
  // }
  xSteps = -xSteps;

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

void loop() {
//   if (!xStepper.run()) {   // run() returns true as long as the final position has not been reached and speed is not 0.
//  }
}
