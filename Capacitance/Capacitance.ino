#include "math.h"  // need this for log()

//Note the filter is the Resistor and Capacitor
#define FILTER_IN_PIN 11
#define FILTER_OUT_PIN A0

void setup() // setup function
{
  Serial.begin(9600);  // set up serial connection at 9600 Baud
  pinMode(FILTER_IN_PIN, OUTPUT);  // setup pins
  digitalWrite(FILTER_IN_PIN, HIGH);  // initially set input as high
  pinMode(FILTER_OUT_PIN,INPUT);
}

void loop() // main loop
{
  float tau = fsmComputeTau();  // FSM to compute tau
//  Serial.print("tau:");
//  Serial.println(tau);
}

// function to compute tau
float computeTau(float measuredTime, float beta)
{ 
  float tau = measuredTime/log((1-beta)/beta);
  return(tau);
}

// functions that returns voltage on pin
float pinVoltage(int pin)
{
  float v_pin = 5.0 * ( (float) analogRead(pin) / 1024.0 );
  return(v_pin);
}

float fsmComputeTau() // state machine to compute tau
{
  static int state = 0;
  static float tau=0;
  static unsigned long startTime = millis();  // for time in state
  unsigned long currentTime;
  float capVoltage = pinVoltage(FILTER_OUT_PIN);  // voltage across capacitor
  Serial.println(capVoltage);  // plot voltage vs. time
  float Vcc = 5;  // pin voltage when HIGH
  float beta = 0.1;  // beta: switching threshold
  
  switch (state) 
  {
    case 0: // charging capacitor (pin is HIGH)
      if (capVoltage >= (1-beta)*Vcc){
        state = 1; // next state
        currentTime = millis();
        float T = ((float)(currentTime-startTime))/1000.0;
        startTime = currentTime;
        tau = computeTau(T, beta);  
        digitalWrite(FILTER_IN_PIN, LOW);
      }
      break;
    
    case 1: // discharging capacitor (pin is Low)
      if (capVoltage <= beta*Vcc){
        state = 0; // next state
        currentTime = millis();
        float T = ((float)(currentTime-startTime))/1000.0;
        startTime = currentTime;
        tau = computeTau(T, beta);
        digitalWrite(FILTER_IN_PIN, HIGH);
      }
      break;
    default:
      state=0;
      break;
  }
  return(tau);
}
