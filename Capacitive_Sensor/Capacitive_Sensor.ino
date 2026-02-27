#include "Arduino.h"
#include <CapacitiveSensor.h>
#define CAP_SENSOR_SEND     11
#define CAP_SENSOR_RECEIVE  7
#define CAP_SENSOR_SAMPLES 40
#define CAP_SENSOR_TAU_THRESHOLD 25

void setup() {
  Serial.begin(9600);  
  pinMode(CAP_SENSOR_RECEIVE, INPUT);
  pinMode(CAP_SENSOR_SEND, OUTPUT);  
}
void loop() 
{
   static CapacitiveSensor sensor 
    = CapacitiveSensor(CAP_SENSOR_SEND, CAP_SENSOR_RECEIVE);
  long tau 
    =  sensor.capacitiveSensor(CAP_SENSOR_SAMPLES); 
  Serial.println(tau);
}
