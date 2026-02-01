#include <FastAccelStepper.h>

class StepperMotor {
private:
    // reference to the fast accel stepper engine controller
    FastAccelStepperEngine& stepperEngine;

    FastAccelStepper* motor;
    int stepPin, dirPin, limitPin;
    uint32_t normalMaxSpeed;
    uint32_t normalAcceleration;
    uint32_t calibrationMaxSpeed;
    uint32_t calibrationAcceleration;
public:
    StepperMotor();

    void initializeMotor(int stepPin, int dirPin, int limitPin, uint32_t normalSpeed, uint32_t normalAccel, uint32_t calibSpeed, uint32_t calibAccel, FastAccelStepperEngine& engineRef);
    void setNormalMotorSettings();
    void setCalibrationMotorSettings();
    void setCustomMotorSpeedAccel(uint32_t speed, uint32_t accel);
    void calibrate();
    void move(long position);
    void moveTo(long position);
    void setZeroPosition();
    int32_t getCurrentPosition();
    void waitforCompletion();
    void moveToAndWaitForCompletion(long position);
    void setLinear();
};