#include <AccelStepper.h>

// in mm
const int ArmSegment1Length = 260;
const int ArmSegment2Length = 170;

// X Axis step pins
const int xStepPin = 2;
const int xDirPin = 5;

// Y Axis step pins
const int yStepPin = 3;
const int yDirPin = 6;

// Z Axis step pins
const int zStepPin = 4;
const int zDirPin = 7;

const int stepsPerRevolution = 800;  // Adjust for quarter-stepping


/* */
// Limit switch pinout constants
const int xLimitPin = 9;
const int yLimitPin = 10;
const int zLimitPin = 11;
/* */

const int BaseGearReductionRatio = 5.5;
const int ArmGearReductionRatio = 27.5;

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
        
        motor.moveTo(-20000);  // Move far in reverse
  
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
    
};

StepperMotor xStepperMotor(xStepPin, xDirPin, baseStepperSpeed, baseStepperAccel, baseStepperSpeed / 10, baseStepperAccel / 2);
StepperMotor yStepperMotor(yStepPin, yDirPin, baseStepperSpeed * 5, baseStepperAccel, 1250.0 , baseStepperAccel);
StepperMotor zStepperMotor(zStepPin, zDirPin, baseStepperSpeed, baseStepperAccel, baseStepperSpeed / 4, baseStepperAccel / 2.5); 

AccelStepper xStepper(AccelStepper::DRIVER, xStepPin, xDirPin);
AccelStepper yStepper(AccelStepper::DRIVER, yStepPin, yDirPin);
AccelStepper zStepper(AccelStepper::DRIVER, zStepPin, zDirPin);

void setup() {
  delay(3000);

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

  // ensure arm clearance for y axis calibration routine
  zStepperMotor.setNormalMotorSettings();
  zStepperMotor.moveTo(5000);

  // calibrate all X,Y,Z starting positions
  runCalibrationRoutine();
}

void runCalibrationRoutine() {
  xStepperMotor.calibrate(xLimitPin);
  xStepperMotor.moveTo(1500);

  yStepperMotor.calibrate(yLimitPin);
  yStepperMotor.moveTo(10000);

  zStepperMotor.calibrate(zLimitPin);
  zStepperMotor.moveTo(5000);
}

void loop() {
//   if (!xStepper.run()) {   // run() returns true as long as the final position has not been reached and speed is not 0.
//  }
}
