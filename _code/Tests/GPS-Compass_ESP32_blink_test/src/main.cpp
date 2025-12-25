#include <Arduino.h>

// LED blink on ESP32-S3 pin 10
const int LED_PIN = 10;
const unsigned long ON_MS = 500;
const unsigned long OFF_MS = 500;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  delay(ON_MS);
  digitalWrite(LED_PIN, LOW);
  delay(OFF_MS);
}