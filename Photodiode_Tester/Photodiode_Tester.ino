#include "Arduino.h"

#define PD_IN_PIN A7
#define LED_PIN LED_BUILTIN

// Voltage at which a light is considered to be present
#define PHOTODIODE_LIGHT_THRESHOLD 3

void setup() {
  Serial.begin(9600);
    pinMode(PD_IN_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
}

void loop() {
    if (isLight(PD_IN_PIN)) {
        digitalWrite(LED_PIN, HIGH);
    } else {
        digitalWrite(LED_PIN, LOW);
    }
}

// Function that detects if light is present
bool isLight(int pin) {
  float light = getPinVoltage(pin);
  Serial.println(light); // Use this line to test
  return (light > PHOTODIODE_LIGHT_THRESHOLD);
}

float getPinVoltage(int pin) {
    return 5 * (float)analogRead(pin) / 1024;
}
