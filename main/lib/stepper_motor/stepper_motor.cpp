#include "stepper_motor.h"


void StepperMotor::initializeMotor(int stepPin, int dirPin, int limitPin,
                                   uint32_t normalSpeed, uint32_t normalAccel,
                                   uint32_t calibSpeed, uint32_t calibAccel,
                                   FastAccelStepperEngine& engineRef) {
        
        this->stepPin = stepPin;
        this->dirPin = dirPin;
        this->limitPin = limitPin;
        this->normalMaxSpeed = normalSpeed;
        this->normalAcceleration = normalAccel;
        this->calibrationMaxSpeed = calibSpeed;
        this->calibrationAcceleration = calibAccel;
        this->stepperEngine = engineRef;

        Serial.print("Connecting stepper to pin: ");
        Serial.println(stepPin);
        
        motor = stepperEngine.stepperConnectToPin(stepPin);
        if (motor == nullptr) {
            Serial.println("ERROR: Failed to connect stepper!");
            return;
        }
        
        if (dirPin >= 0) {  // Only set if valid pin
            motor->setDirectionPin(dirPin);
        }
        
        setNormalMotorSettings();
}

void StepperMotor::setNormalMotorSettings() {
    motor->setSpeedInHz(normalMaxSpeed);
    motor->setAcceleration(normalAcceleration);
}

void StepperMotor::setCalibrationMotorSettings() {
    motor->setSpeedInHz(calibrationMaxSpeed);
    motor->setAcceleration(calibrationAcceleration);
}

void StepperMotor::setCustomMotorSpeedAccel(uint32_t speed, uint32_t accel) {
    motor->setSpeedInHz(speed);
    motor->setAcceleration(accel);
}

void StepperMotor::calibrate() {
    setCalibrationMotorSettings();

    motor->runBackward();

    while (digitalRead(limitPin) == LOW) {
        delay(10);
    }

    motor->forceStop();
    delay(100);

    Serial.println("Axis Calibrated!");
    setZeroPosition();

    // reset speed and accel settings
    setNormalMotorSettings();

    delay(500);
}

// moves relative to current position
void StepperMotor::move(long position) {
    motor->move(position);
}

void StepperMotor::moveTo(long position) {
    motor->moveTo(position);
}

// set zero position after calibration
void StepperMotor::setZeroPosition() {
    motor->setCurrentPosition(0);
}

int32_t StepperMotor::getCurrentPosition() {
    return motor->getCurrentPosition();
}

void StepperMotor::waitforCompletion() {
    while (motor && motor->isRunning()) {
    delay(10);
    }
}

void StepperMotor::moveToAndWaitForCompletion(long position) {
    moveTo(position);
    waitforCompletion();
}

void StepperMotor::setLinear() {
    motor->setLinearAcceleration(0);
}