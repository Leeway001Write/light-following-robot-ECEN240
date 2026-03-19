#include "Arduino.h"
#include <Servo.h>  // loads the Servo library

// Servo pin
#define SERVO_PIN 9

// Parameters for servo control as well as instantiation
#define SERVO_START_ANGLE 45
#define SERVO_UP_LIMIT 180
#define SERVO_DOWN_LIMIT 0
static Servo myServo;

void setup() {
  Serial.begin(9600);  // set up serial connection at 9600 Baud

  //Set up servo
  myServo.attach(SERVO_PIN);
  myServo.write(SERVO_START_ANGLE);
  delay(1000);
}

void loop() { // Replace your old void loop with this
//  MoveServo(); // state/ machine to move servo back and forth
}

void MoveServo() {
    static int state = 0;
    static int servoAngle = SERVO_START_ANGLE;
    Serial.println(servoAngle);
    
    switch(state) {
    case 0: // servo moving in positive direction
      servoAngle += 1;
      if (servoAngle >= SERVO_UP_LIMIT) {
        delay(100);
        state = 1;
      }
      break;
    case 1: // servo moving in negative direction
      servoAngle -= 1;
      if (servoAngle <= SERVO_DOWN_LIMIT) {
        delay(100);
        state = 0;
      }
      break;
  }
  myServo.write(servoAngle); // send angle to the servo 
  // the .write() function expects an integer between 0 and 180 degrees
}
