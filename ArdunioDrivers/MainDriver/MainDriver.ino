#include <Servo.h>
#include <Arduino.h>

Servo servo1;  // create servo object to control a servo


int pos = 0;  // variable to store the servo position

void testRun() {
  delay(500);

  for (pos = 110; pos >= 55; pos -= 1) {  // goes from 180 degrees to 0 degrees

    servo1.write(pos);  // tell servo to go to position in variable 'pos'

    delay(100);  // waits 15ms for the servo to reach the position
  }

  digitalWrite(7, HIGH);
  delay(1000);

  for (pos = 55; pos <= 110; pos += 1) {  

    servo1.write(pos); 

    delay(100);  // waits 15ms for the servo to reach the position
  }

  digitalWrite(7, LOW);
  delay(1000);

}

void setup() {

  pinMode(7, OUTPUT);

  servo1.attach(8);  // attaches the servo on pin 9 to the servo object

  //digitalWrite(7, HIGH);
  testRun();
}


void loop() {
  // digitalWrite(7, HIGH);
  // delay(3000);
  // digitalWrite(7, LOW);
  // delay(3000);

// for (pos = 55; pos <= 110; pos += 1) { // goes from 0 degrees to 180 degrees

//     // in steps of 1 degree

//     servo1.write(pos);              // tell servo to go to position in variable 'pos'

//     delay(15);                       // waits 15ms for the servo to reach the position

//   }

//   for (pos = 110; pos >= 55; pos -= 1) { // goes from 180 degrees to 0 degrees

//     servo1.write(pos);              // tell servo to go to position in variable 'pos'

//     delay(15);                       // waits 15ms for the servo to reach the position

//   }
}