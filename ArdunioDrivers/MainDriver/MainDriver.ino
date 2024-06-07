#include <Servo.h>
#include <Arduino.h>

Servo baseServo;
Servo mainArmServo;
Servo leverArmServo;

int pos = 0;  // variable to store the servo position

// void testRun() {
//   delay(500);

//   for (pos = 110; pos >= 55; pos -= 1) {  // goes from 180 degrees to 0 degrees

//     servo1.write(pos);  // tell servo to go to position in variable 'pos'

//     delay(100);  // waits 15ms for the servo to reach the position
//   }

//   digitalWrite(7, HIGH);
//   delay(1000);

//   for (pos = 55; pos <= 110; pos += 1) {  

//     servo1.write(pos); 

//     delay(100);  // waits 15ms for the servo to reach the position
//   }

//   digitalWrite(7, LOW);
//   delay(1000);

// }

// void testBase() {
//     delay(1000);
//     double desiredAngle = 63.43;  // Target angle in degrees
//     double gearRatio = 2.27;  // Gear ratio
//     double motorAngle = desiredAngle / gearRatio;

//     int currentPos = baseServo.read();  // Read current position of servo
//     int targetPos = int(180 - motorAngle);  // Calculate target position considering gear inversion

//     if (currentPos < targetPos) {
//         for (pos = currentPos; pos <= targetPos; pos++) {  // Increment position
//             baseServo.write(pos);
//             delay(15);  // Short delay to allow the servo to reach the position
//         }
//     } else {
//         for (pos = currentPos; pos >= targetPos; pos--) {  // Decrement position
//             baseServo.write(pos);
//             delay(15);  // Short delay to allow the servo to reach the position
//         }
//     }
// }

void testMainArm() {
    int targetPos = 86;

    targetPos = int(180 - targetPos);
    int currentPos = mainArmServo.read();  // Read current position of servo

    if (currentPos < targetPos) {
        for (pos = currentPos; pos <= targetPos; pos++) {  // Increment position
            mainArmServo.write(pos);
            delay(15);  // Short delay to allow the servo to reach the position
        }
    } else {
        for (pos = currentPos; pos >= targetPos; pos--) {  // Decrement position
            mainArmServo.write(pos);
            delay(15);  // Short delay to allow the servo to reach the position
        }
    }
}

void testLeverArm() {
    int targetPos = 100;

    int currentPos = leverArmServo.read();  // Read current position of servo

    if (currentPos < targetPos) {
        for (pos = currentPos; pos <= targetPos; pos++) {  // Increment position
            leverArmServo.write(pos);
            delay(15);  // Short delay to allow the servo to reach the position
        }
    } else {
        for (pos = currentPos; pos >= targetPos; pos--) {  // Decrement position
            leverArmServo.write(pos);
            delay(15);  // Short delay to allow the servo to reach the position
        }
    }
}

// in mm
int mainVertArmLength = 135;
int mainHoriArmLength = 145;

// length from the servo's pivot to the connection point on the lever arm
int leverLength = 56;

// length from the lever arm's endpoint through the linkage to Joint 2
int linkLength = 135;

void positionDriver(int x, int y, int z) {

}


void setup() {

  pinMode(7, OUTPUT);

  baseServo.attach(8);  // attaches the servo on pin 9 to the servo object
  mainArmServo.attach(9);  // attaches the servo on pin 9 to the servo object
  leverArmServo.attach(10);  // attaches the servo on pin 9 to the servo object
  //digitalWrite(7, HIGH);
  //testRun();
  //testBase();
  testMainArm();
  testLeverArm();
}




void loop() {
  // digitalWrite(7, HIGH);
  // delay(3000);
  // digitalWrite(7, LOW);
  // delay(3000);

// for (pos = 55; pos <= 110; pos += 1) { // goes from 0 degrees to 180 degrees

//     // in steps of 1 degree

//     baseServo.write(pos);              // tell servo to go to position in variable 'pos'

//     delay(15);                       // waits 15ms for the servo to reach the position

//   }

//   for (pos = 110; pos >= 55; pos -= 1) { // goes from 180 degrees to 0 degrees

//     baseServo.write(pos);              // tell servo to go to position in variable 'pos'

//     delay(15);                       // waits 15ms for the servo to reach the position

//   }
}