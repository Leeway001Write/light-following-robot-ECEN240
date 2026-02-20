#include "Arduino.h"

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
float BatteryVoltage;

void setup() {
    pinMode(BATTERY_PIN, INPUT);
    pinMode(BATTERY_LED_1, OUTPUT);
    pinMode(BATTERY_LED_2, OUTPUT);
    pinMode(BATTERY_LED_3, OUTPUT);

    Serial.begin(9600);
}

void loop() {
    // PERCEPTION
    BatteryVoltage = getBatteryVoltage();
    Serial.print("Battery_Voltage:");
    Serial.println(V);

    // PLANNING

    // ACTION
    fsmUpdateBatteryMonitor();
}

void fsmUpdateBatteryMonitor() {
    static int batteryState = 0;

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
                batteryState = BATTERY_MAX_BATTERY_V;
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

float getBatteryVoltage() {
    return getPinVoltage(BATTERY_PIN);
}

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

float getPinVoltage(int pin) {
  return V_IN * (float)analogRead(pin) / 1024;
}
