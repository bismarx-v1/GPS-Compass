#include <Arduino.h>
#define LED_PIN 13
#define IO42 42

// LED blink on ESP32-S3 pin 21
const unsigned long ON_MS = 500;
const unsigned long OFF_MS = 500;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(IO42, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(IO42, HIGH);
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  delay(ON_MS);
  digitalWrite(LED_PIN, LOW);
  delay(OFF_MS);
  digitalWrite(IO42, HIGH);
  delay(ON_MS);
  digitalWrite(IO42, LOW);
  delay(OFF_MS);
}