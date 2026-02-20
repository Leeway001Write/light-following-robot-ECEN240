#include "Arduino.h"

// Pin definitions
#define BATTERY_PIN A7
#define LED_1 12
#define LED_2 11
#define LED_3 10

#define FULL 5.0 // Max battery voltage (V)
#define V_IN 5.0 // True voltage into 5V pin

// Battery State definitions
#define BATTERY_FULL 0
#define BATTERY_MEDIUM 1
#define BATTERY_LOW 2
#define BATTERY_REPLACE 3

// Global Battery Voltage
float V;

void setup() {
    pinMode(BATTERY_PIN, INPUT);
    pinMode(LED_1, OUTPUT);
    pinMode(LED_2, OUTPUT);
    pinMode(LED_3, OUTPUT);

    Serial.begin(9600);
}

void loop() {
    // PERCEPTION
    V = getBatteryVoltage();
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
            onLED(LED_1);
            onLED(LED_2);
            onLED(LED_3);

            // Transition to MEDIUM if charge is below 90%
            if (V < (0.9 * FULL)) {
                batteryState = BATTERY_MEDIUM;
            }
        break;
        
        case BATTERY_MEDIUM:
            // Turn 2 LEDS on
            onLED(LED_1);
            onLED(LED_2);
            offLED(LED_3);

            // Transition to FULL if charge is at least 90%
            if (V >= (0.9 * FULL)) {
                batteryState = BATTERY_FULL;
            }
            // Transition to LOW  if charge is below    80%
            if (V < (0.8 * FULL)) {
                batteryState = BATTERY_LOW;
            }
        break;

        case BATTERY_LOW:
            // Turn 1 LED on
            onLED(LED_1);
            offLED(LED_2);
            offLED(LED_3);

            // Transition to MEDIUM  if charge is at least 80%
            if (V >= (0.8 * FULL)) {
                batteryState = BATTERY_MEDIUM;
            }
            // Transition to REPLACE if charge is below    70%
            if (V < (0.7 * FULL)) {
                batteryState = BATTERY_REPLACE;
            }
        break;

        case BATTERY_REPLACE:
            // Turn 0 LEDS on
            offLED(LED_1);
            offLED(LED_2);
            offLED(LED_3);
            
            // Transition to LOW if charge is at least 0.7
            if (V >= (0.7 * FULL)) {
                batteryState = BATTERY_LOW;
            }
        break;
    }
}

float getBatteryVoltage() {
    return getPinVoltage(BATTERY_PIN);
}

void onLED(int pin) {
    digitalWrite(pin, HIGH);
}
void offLED(int pin) {
    digitalWrite(pin, LOW);
}

float getPinVoltage(int pin) {
  return V_IN * (float)analogRead(pin) / 1024;
}
