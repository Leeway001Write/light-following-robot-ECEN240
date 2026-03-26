/********************************************************************
  ECEN 240/301 Lab Code
  Light-Following Robot

  The approach of this code is to use an architectured that employs
  three different processes:
    Perception
    Planning
    Action

  By separating these processes, this allows one to focus on the
  individual elements needed to do these tasks that are general
  to most robotics.


  Version History
  1.1.3       11 January 2023   Creation by Dr. Mazzeo and TAs from 2022 version

 ********************************************************************/

/* These initial includes allow you to use necessary libraries for
your sensors and servos. */
#include "Arduino.h"
#include <CapacitiveSensor.h>
#include <NewPing.h>
#include <Servo.h>

//
// Compiler defines: the compiler replaces each name with its assignment
// (These make your code so much more readable.)
//

/***********************************************************/
// Hardware pin definitions
// Replace the pin numbers with those you connect to your robot

// LED pins (note that digital pins do not need "D" in front of them)
#define LED_1   6       // Far Left LED - Servo Up
#define LED_3   4       // Middle LED - Collision
#define LED_5   2       // Far Right LED - Servo Down

// Motor enable pins - Lab 3
#define H_BRIDGE_ENA 5
#define H_BRIDGE_ENB 3

// Battery Monitor
  int EnableBatteryMonitor = false;
  
  // Pin definitions
  #define BATTERY_PIN A7
  #define BATTERY_LED_1 12
  #define BATTERY_LED_2 11
  #define BATTERY_LED_3 10
  
  #define MAX_BATTERY_V 5.0 // Max battery voltage (V)
  #define V_IN 5.0 // True voltage into 5V pin
  
  // Battery State definitions
  #define BATTERY_FULL 0
  #define BATTERY_MEDIUM 1
  #define BATTERY_LOW 2
  #define BATTERY_REPLACE 3
  
  // Global Battery Voltage
  float BatteryVoltage = 0;

// Photodiode pins - Lab 5
#define PHOTODIODE_PIN_UP     A2     // Photodiode - Servo Up     ORANGE
#define PHOTODIODE_PIN_LEFT   A3     // Photodiode - Left Motor   PURPLE
#define PHOTODIODE_PIN_RIGHT  A5     // Photodiode - Right Motor  YELLOW
#define PHOTODIODE_PIN_DOWN   A6     // Photodiode - Servo Down   BLUE

// Capacitive sensor pins - Lab 4
  #define CAP_SENSOR_SEND_PIN 11
  #define CAP_SENSOR_RECEIVE_PIN 7

// Collision sensor pins - Lab 6
  #define TRIGGER_PIN 12  // Arduino pin tied to trigger pin on the ultrasonic sensor.
  #define ECHO_PIN 10  // Arduino pin tied to echo pin on the ultrasonic sensor.

// Servo pin - Lab 6
#define SERVO_PIN 9


/***********************************************************/
// Configuration parameter definitions
// Replace the parameters with those that are appropriate for your robot

// Voltage at which a button is considered to be pressed
#define BUTTON_THRESHOLD 2.5

// Voltage at which a photodiode voltage is considered to be present - Lab 5
#define PHOTODIODE_LIGHT_THRESHOLD 3 // Sensor is ON if light is greater than this many volts

// Number of samples that the capacitor sensor will use in a measurement - Lab 4
#define CAP_SENSOR_SAMPLES 40
#define CAP_SENSOR_TAU_THRESHOLD 1000 // tau value at which touch will register

// Parameters for servo control as well as instantiation - Lab 6
#define SERVO_START_ANGLE 135
#define SERVO_UP_LIMIT 110
#define SERVO_DOWN_LIMIT 180
#define SERVO_SPEED 1
static Servo servo;

// Parameters for ultrasonic sensor and instantiation - Lab 6
  // Maximum distance we want to ping for (in centimeters). 
  // Maximum sensor distance is rated at 400-500cm, so we choose 200.
  #define MAX_DISTANCE 200 

  // NewPing setup of pins
  static NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); 


// Parameter to define when the ultrasonic sensor detects a collision - Lab 6
  #define COLLISION_DISTANCE 40 // cm



/***********************************************************/
// Defintions that allow one to set states
// Sensor state definitions
#define DETECTION_NO    0
#define DETECTION_YES   1

// Motor speed definitions - Lab 4
#define SPEED_STOP      0
#define SPEED_LOW       (int) (255 * 0.45)
#define SPEED_MED       (int) (255 * 0.75)
#define SPEED_HIGH      (int) (255 * 1)

// Collision definitions
#define COLLISION_ON   0
#define COLLISION_OFF  1

// Driving direction definitions
#define DRIVE_STOP      0
#define DRIVE_LEFT      1
#define DRIVE_RIGHT     2
#define DRIVE_STRAIGHT  3

// Servo movement definitions
#define SERVO_MOVE_STOP 0
#define SERVO_MOVE_UP   1
#define SERVO_MOVE_DOWN 2


/***********************************************************/
// Global variables that define PERCEPTION and initialization

// Collision (using Definitions)
int SensedCollision = DETECTION_NO;
float SensedDistance = COLLISION_DISTANCE;

// Photodiode inputs (using Definitions) - The button represent the photodiodes for lab 2
int SensedLightRight = DETECTION_NO;
int SensedLightLeft = DETECTION_NO;
int SensedLightUp = DETECTION_NO;
int SensedLightDown = DETECTION_NO;

// Capacitive sensor input (using Definitions) - Lab 4
int SensedCapacitiveTouch = DETECTION_NO;


/***********************************************************/
// Global variables that define ACTION and initialization

// Collision Actions (using Definitions)
int ActionCollision = COLLISION_OFF;

// Main motors Action (using Definitions)
int ActionRobotDrive = DRIVE_STOP;
// Add speed action in Lab 4
int ActionRobotSpeed = SPEED_LOW;

// Servo Action (using Definitions)
int ActionServoMove =  SERVO_MOVE_STOP;

/***********************************************************/
// Global Objects
static CapacitiveSensor touchSensor = CapacitiveSensor(CAP_SENSOR_SEND_PIN, CAP_SENSOR_RECEIVE_PIN);

/********************************************************************
  SETUP function - this gets executed at power up, or after a reset
 ********************************************************************/
void setup() {
  //Set up serial connection at 9600 Baud
  Serial.begin(9600);
  
  //Set up output pins
  pinMode(LED_1, OUTPUT);
  pinMode(H_BRIDGE_ENA, OUTPUT);
  pinMode(LED_3, OUTPUT);
  pinMode(H_BRIDGE_ENB, OUTPUT);
  pinMode(LED_5, OUTPUT);

  pinMode(CAP_SENSOR_SEND_PIN, OUTPUT);

  pinMode(TRIGGER_PIN, OUTPUT); // sonar pulse sent out through TRIGGER_PIN    
  
  
  //Set up input pins
  pinMode(PHOTODIODE_PIN_UP, INPUT);
  pinMode(PHOTODIODE_PIN_LEFT, INPUT);
  pinMode(PHOTODIODE_PIN_RIGHT, INPUT);
  pinMode(PHOTODIODE_PIN_DOWN, INPUT);
  
  pinMode(CAP_SENSOR_RECEIVE_PIN, INPUT);
  
  pinMode(ECHO_PIN, INPUT); // sonar return signal read through ECHO_PIN

  //Set up servo - Lab 6
  servo.attach(SERVO_PIN);
  servo.write(SERVO_START_ANGLE);
}

/********************************************************************
  Main LOOP function - this gets executed in an infinite loop until
  power off or reset. - Notice: PERCEPTION, PLANNING, ACTION
 ********************************************************************/
void loop() {
  // This DebugStateOutput flag can be used to easily turn on the
  // serial debugging to know what the robot is perceiving and what
  // actions the robot wants to take.
  int DebugStateOutput = false; // Change false to true to debug
  
  RobotPerception(); // PERCEPTION
  if (DebugStateOutput) {
    Serial.print("Perception:");
    Serial.print(SensedLightUp);
    Serial.print(SensedLightLeft);
    Serial.print(SensedCollision);
    Serial.print(SensedLightRight); 
    Serial.print(SensedLightDown);
   Serial.print(SensedCapacitiveTouch);
    Serial.print("\t");
  }
  
  RobotPlanning(); // PLANNING
  if (DebugStateOutput) {
    Serial.print(" Action:");
    Serial.print(ActionCollision);
    Serial.print(ActionRobotDrive); 
    Serial.print(ActionServoMove);
    Serial.print(" "); Serial.print(ActionRobotSpeed);
    Serial.print("\t");
  }
  RobotAction(); // ACTION
  Serial.print("\n");
}

/**********************************************************************************************************
  Robot PERCEPTION - all of the sensing
 ********************************************************************/
void RobotPerception() {
  // This function polls all of the sensors and then assigns sensor outputs
  // that can be used by the robot in subsequent stages


  
  // Photodiode Sensing
  //Serial.print(getPinVoltage(BUTTON_2)); Serial.print("\t"); //uncomment for debugging
  
  if (isLight(PHOTODIODE_PIN_LEFT)){
    SensedLightLeft = DETECTION_YES;
  } else {
    SensedLightLeft = DETECTION_NO;
  }
  if (isLight(PHOTODIODE_PIN_RIGHT)) { 
    SensedLightRight = DETECTION_YES;
  } else {
    SensedLightRight = DETECTION_NO;
  }

      
  /* Add code to detect if light is up or down. Lab 2 milestone 3*/
  if (isLight(PHOTODIODE_PIN_UP)) {
    SensedLightUp = DETECTION_YES;
  } else {
    SensedLightUp = DETECTION_NO;
  }
  if (isLight(PHOTODIODE_PIN_DOWN)) {
    SensedLightDown = DETECTION_YES;
  } else {
    SensedLightDown = DETECTION_NO;
  }
  

   // Capacitive Sensor
   if (isCapacitiveSensorTouched()) {
    SensedCapacitiveTouch = DETECTION_YES;
   } else {
    SensedCapacitiveTouch = DETECTION_NO;
   }

   // Collision Sensor
   if (isCollision()) {
    SensedCollision = DETECTION_YES;
   } else {
    SensedCollision = DETECTION_NO;
   }

   // Battery Monitor
   if (EnableBatteryMonitor) {
    BatteryVoltage = getPinVoltage(BATTERY_PIN);
   }
}


////////////////////////////////////////////////////////////////////
// Function to read pin voltage
////////////////////////////////////////////////////////////////////
float getPinVoltage(int pin) {
  //This function can be used for many different tasks in the labs
  //Study this line of code to understand what is going on!!
  //What does analogRead(pin) do?
  //Why is (float) needed?
  //Why divide by 1024?
  //Why multiply by 5?
  return 5 * (float)analogRead(pin) / 1024;
}

////////////////////////////////////////////////////////////////////
// Function to determine if a button is pushed or not
////////////////////////////////////////////////////////////////////
bool isButtonPushed(int button_pin) {
  //This function can be used to determine if a said button is pushed.
  //Remember that when the voltage is 0, it's only close to zero.
  //Hint: Call the getPinVoltage function and if that value is greater
  // than the BUTTON_THRESHOLD variable toward the top of the file, return true.
  if (getPinVoltage(button_pin) >= BUTTON_THRESHOLD){
    return true;
  } else {
    return false;
  }
}


////////////////////////////////////////////////////////////////////
// Function that detects if there is an obstacle in front of robot
////////////////////////////////////////////////////////////////////
bool isCollision() {
  int sonar_distance = getDistanceSmoothed(); // If the distance is too big, it returns 0.
   Serial.println(sonar_distance);

  if(sonar_distance != 0){ 
    return (sonar_distance < COLLISION_DISTANCE);
  } else {
	  return false;
  }
}

////////////////////////////////////////////////////////////////////
// Function that detects if the capacitive sensor is being touched
////////////////////////////////////////////////////////////////////
bool isCapacitiveSensorTouched() {
  long tau = touchSensor.capacitiveSensor(CAP_SENSOR_SAMPLES); 
  
  return tau >= CAP_SENSOR_TAU_THRESHOLD;
}

////////////////////////////////////////////////////////////////////
// Function that detects if light is present
////////////////////////////////////////////////////////////////////
bool isLight(int pin) {
  float light = getPinVoltage(pin);
  // Serial.println(light); // Use this line to test
  return (light > PHOTODIODE_LIGHT_THRESHOLD);
}


/**********************************************************************************************************
  Robot PLANNING - using the sensing to make decisions
 **********************************************************************************************************/
void RobotPlanning(void) {
  // The planning FSMs that are used by the robot to assign actions
  // based on the sensing from the Perception stage.
  fsmCollisionDetection(); // Milestone 1
  fsmMoveServoUpAndDown(); // Milestone 3
  
  fsmCapacitiveSensorSpeedControl();

  if (EnableBatteryMonitor) {
    fsmUpdateBatteryMonitor(); // Lab 3
  }
}

////////////////////////////////////////////////////////////////////
// State machine for detecting collisions, and stopping the robot
// if necessary.
////////////////////////////////////////////////////////////////////
void fsmCollisionDetection() {
  static int collisionDetectionState = 0;
  static float initialCollisionDistance = 0;
  //Serial.print(collisionDetectionState); Serial.print("\t"); //uncomment for debugging
  
  switch (collisionDetectionState) {
    case 0: //collision detected
      //There is an obstacle, stop the robot
      ActionCollision = COLLISION_ON; // Sets the action to turn on the collision LED
      ActionRobotDrive = DRIVE_STOP;
      
      //State transition logic
      initialCollisionDistance = getDistanceSmoothed();
      collisionDetectionState = 2; // Turn away
      break;
    
    case 1: //no collision
      //There is no obstacle, drive the robot
      ActionCollision = COLLISION_OFF; // Sets action to turn off the collision LED

      fsmSteerRobot(); // Milestone 2
      
      //State transition logic
      if (isCollision()) {
        collisionDetectionState = 0; //if collision, go to collision state
      }
      break;

    case 2: //turning away (left)
      // There is an obstacle, turn left
      ActionRobotDrive = DRIVE_LEFT;

      //State transition logic
      if ( SensedCollision == DETECTION_NO) {
        collisionDetectionState = 1; //if no collision, go to no collision state
      } else if ((initialCollisionDistance - getDistanceSmoothed()) > (COLLISION_DISTANCE / 3)) {
        collisionDetectionState = 3; //if turning left is too tight (sensor is on left), turn away to the right instead
      }
      break;
    
    case 3: //turning away (right)
      // There is an obstace, turn right
      ActionRobotDrive = DRIVE_RIGHT;

      //State transition logic
      if ( SensedCollision == DETECTION_NO) {
        collisionDetectionState = 1; //if no collision, go to no collision state
      }
      break;
      
    default: // error handling
      {
        collisionDetectionState = 0;
      }
      break;
  }
}

////////////////////////////////////////////////////////////////////
// State machine for detecting if light is to the right or left,
// and steering the robot accordingly.
////////////////////////////////////////////////////////////////////
void fsmSteerRobot() {
  static int steerRobotState = 0;
  //Serial.print(steerRobotState); Serial.print("\t"); //uncomment for debugging
  
  switch (steerRobotState) {
    case 0: //light is not detected
      ActionRobotDrive = DRIVE_STOP;
      
      //State transition logic
      if (SensedLightLeft == DETECTION_YES)  {
        steerRobotState = 1; //if light on left of robot, go to left state
      } else if (SensedLightRight == DETECTION_YES) {
        steerRobotState = 2; //if light on right of robot, go to right state
      }
      break;
    
    case 1: //light is to the left of robot
      //The light is on the left, turn left
      ActionRobotDrive = DRIVE_LEFT;
      
      //State transition logic
      if (SensedLightRight == DETECTION_YES) {
        steerRobotState = 3; //if light is on right, then go straight
      } else if (SensedLightLeft == DETECTION_NO) {
        steerRobotState = 0; //if light is not on left, go back to stop state
      }
      
      break;
    
    case 2: //light is to the right of robot
      //The light is on the right, turn right
      ActionRobotDrive = DRIVE_RIGHT;
      
      //State transition logic
      if (SensedLightLeft == DETECTION_YES) {
        steerRobotState = 3; //if light is on left, then go straight
      } else if (SensedLightRight == DETECTION_NO) {
        steerRobotState = 0; //if light is not on right, go back to stop state
      }

      break;
      
    case 3: // light is on both right and left
      ActionRobotDrive = DRIVE_STRAIGHT;

      // State transition logic
      if (SensedLightLeft == DETECTION_NO) {
        steerRobotState = 2; //if light is not on left, go to right state
      } else if (SensedLightRight == DETECTION_NO) {
        steerRobotState = 1; //if light is not on right, go to left state
      }

      break;
      
    default: // error handling
    {
      steerRobotState = 0;
    }
  }
}

////////////////////////////////////////////////////////////////////
// State machine for detecting if light is above or below center,
// and moving the servo accordingly.
////////////////////////////////////////////////////////////////////
void fsmMoveServoUpAndDown() {
  static int moveServoState = 0;
  //Serial.print(moveServoState); Serial.print("\t"); //uncomment for debugging
  
  //Create a state machine modeled after the ones in milestones 1 and 2
  // to plan the servo action based off of the perception of the robot
  //Remember no light or light in front = servo doesn't move
  //Light above = servo moves up
  //Light below = servo moves down
  switch (moveServoState) {
    case 0: // No light / light is in center
        // Don't move
        ActionServoMove = SERVO_MOVE_STOP;

        // Check light sensors
        if (SensedLightUp && !SensedLightDown) {
            moveServoState = 1;
        } else if (SensedLightDown && !SensedLightUp) {
            moveServoState = 2;
        }
        break;

    case 1: // Light sensed up
        // UP
        ActionServoMove = SERVO_MOVE_UP;

        // Check light sensors
        if (!(SensedLightUp ^ SensedLightDown)) {
            moveServoState = 0;
        }
        break;

    case 2: // Light sensed down
        // DOWN
        ActionServoMove = SERVO_MOVE_DOWN;
        
        // Check light sensors
        if (!(SensedLightUp ^ SensedLightDown)) {
            moveServoState = 0;
        }
        break;
  }  
}

////////////////////////////////////////////////////////////////////
// State machine for detecting when the capacitive sensor is
// touched, and changing the robot's speed.
////////////////////////////////////////////////////////////////////
void fsmCapacitiveSensorSpeedControl() {
  static int sensorState = 0;

  switch (sensorState) {
    case 0: // Not pressed
      doTurnLedOff(LED_BUILTIN);

      if (SensedCapacitiveTouch == DETECTION_YES) {
        sensorState = 1;
      }
      break;
    case 1: // Pressed
      doTurnLedOn(LED_BUILTIN);

      if (SensedCapacitiveTouch == DETECTION_NO) {
        sensorState = 2;
      }
      break;
    case 2: // Released
      fsmChangeSpeed();  

      sensorState = 0;
      break;
  }
}

////////////////////////////////////////////////////////////////////
// State machine for cycling through the robot's speeds.
////////////////////////////////////////////////////////////////////
void fsmChangeSpeed() {
  static int speedState = 0;

  switch (speedState) {
    case 0: // STOP
      ActionRobotSpeed = SPEED_STOP;

      speedState = 1;
      break;
    case 1: // LOW
      ActionRobotSpeed = SPEED_LOW;

      speedState = 2;
      break;
    case 2: // MED
      ActionRobotSpeed = SPEED_MED;

      speedState = 3;
      break;
    case 3: // HIGH
      ActionRobotSpeed = SPEED_HIGH;

      speedState = 0;
      break;
  }
}

////////////////////////////////////////////////////////////////////
// State machine for indicating battery charge.
////////////////////////////////////////////////////////////////////
void fsmUpdateBatteryMonitor() {
    static int batteryState = 0;
    Serial.print("B_V: ");
    Serial.println(BatteryVoltage);

    Serial.print("B_STATE: ");
    Serial.println(batteryState);

    switch (batteryState) {
        case BATTERY_FULL:
            // Turn 3 LEDS on
            doTurnLedOn(BATTERY_LED_1);
            doTurnLedOn(BATTERY_LED_2);
            doTurnLedOn(BATTERY_LED_3);

            // Transition to MEDIUM if charge is below 90%
            if (BatteryVoltage < (0.9 * MAX_BATTERY_V)) {
                batteryState = BATTERY_MEDIUM;
            }
        break;
        
        case BATTERY_MEDIUM:
            // Turn 2 LEDS on
            doTurnLedOn(BATTERY_LED_1);
            doTurnLedOn(BATTERY_LED_2);
            doTurnLedOff(BATTERY_LED_3);

            // Transition to MAX_BATTERY_V if charge is at least 90%
            if (BatteryVoltage >= (0.9 * MAX_BATTERY_V)) {
                batteryState = BATTERY_FULL;
            }
            // Transition to LOW  if charge is below    80%
            if (BatteryVoltage < (0.8 * MAX_BATTERY_V)) {
                batteryState = BATTERY_LOW;
            }
        break;

        case BATTERY_LOW:
            // Turn 1 LED on
            doTurnLedOn(BATTERY_LED_1);
            doTurnLedOff(BATTERY_LED_2);
            doTurnLedOff(BATTERY_LED_3);

            // Transition to MEDIUM  if charge is at least 80%
            if (BatteryVoltage >= (0.8 * MAX_BATTERY_V)) {
                batteryState = BATTERY_MEDIUM;
            }
            // Transition to REPLACE if charge is below    70%
            if (BatteryVoltage < (0.7 * MAX_BATTERY_V)) {
                batteryState = BATTERY_REPLACE;
            }
        break;

        case BATTERY_REPLACE:
            // Turn 0 LEDS on
            doTurnLedOff(BATTERY_LED_1);
            doTurnLedOff(BATTERY_LED_2);
            doTurnLedOff(BATTERY_LED_3);
            
            // Transition to LOW if charge is at least 0.7
            if (BatteryVoltage >= (0.7 * MAX_BATTERY_V)) {
                batteryState = BATTERY_LOW;
            }
        break;
    }
}


/**********************************************************************************************************
  Robot ACTION - implementing the decisions from planning to specific actions
 ********************************************************************/
void RobotAction() {
  // Here the results of planning are implented so the robot does something

  // This turns the collision LED on and off
  switch(ActionCollision) {
    case COLLISION_OFF:
      doTurnLedOff(LED_3); //Collision LED off
      break;
    case COLLISION_ON:
      doTurnLedOn(LED_3);
      break;
  }
  
  // This drives the main motors on the robot
  switch(ActionRobotDrive) {
    case DRIVE_STOP:
      analogWrite(H_BRIDGE_ENA, 0);
      analogWrite(H_BRIDGE_ENB, 0);
      break;
    case DRIVE_STRAIGHT:
      analogWrite(H_BRIDGE_ENA, ActionRobotSpeed);
      analogWrite(H_BRIDGE_ENB, ActionRobotSpeed);
      break;
    case DRIVE_RIGHT:
      analogWrite(H_BRIDGE_ENA, 0);
      analogWrite(H_BRIDGE_ENB, ActionRobotSpeed);
      break;
    case DRIVE_LEFT:
      analogWrite(H_BRIDGE_ENA, ActionRobotSpeed);
      analogWrite(H_BRIDGE_ENB, 0);
      break;
  }
  
  // This calls a function to move the servo
    MoveServo();       
}


////////////////////////////////////////////////////////////////////
// Function that causes the servo to move up or down.
////////////////////////////////////////////////////////////////////
void MoveServo() {
  // Note that there needs to be some logic in the action of moving
  // the servo so that it does not exceed its range
  static int CurrentServoAngle = SERVO_START_ANGLE;

  switch(ActionServoMove) {
    case SERVO_MOVE_STOP:
      break;
    case SERVO_MOVE_UP:
      CurrentServoAngle -= SERVO_SPEED;
      if (CurrentServoAngle <= SERVO_UP_LIMIT) {
        servo.write(CurrentServoAngle);
      }
      break;
    case SERVO_MOVE_DOWN:
      CurrentServoAngle += SERVO_SPEED;
      if (CurrentServoAngle >= SERVO_DOWN_LIMIT) {
        servo.write(CurrentServoAngle);
      }
      break;
  }
}



/**********************************************************************************************************
  AUXILIARY functions that may be useful in performing diagnostics
 ********************************************************************/
// Function to turn LED on
void doTurnLedOn(int led_pin)
{
  digitalWrite(led_pin, HIGH);
}

// Function to turn LED off
void doTurnLedOff(int led_pin)
{
  digitalWrite(led_pin, LOW);
}

float getDistanceSmoothed() {
    static float distanceSmoothed = sonar.ping_cm();
    float distance = sonar.ping_cm();
    float alpha = 0.9; // alpha-filter constant
    if (distance != 0) {
        // this is an example of a measurement gate:
        // sensor returns a 0 when it times out 
        // (i.e., no measurement) ignore those measurements
            
        // alpha filter all good measurements     
        distanceSmoothed = alpha*distanceSmoothed +(1-alpha)*distance;
    }

    Serial.println(distanceSmoothed);
    
    return(distanceSmoothed);
}
