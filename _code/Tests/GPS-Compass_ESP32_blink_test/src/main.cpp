#include <Arduino.h>
#define IO09 9
#define IO13 13

// LED blink on ESP32-S3 pin 21
const unsigned long ON_MS = 500;
const unsigned long OFF_MS = 500;

void setup() {
  pinMode(IO09, OUTPUT);
  pinMode(IO13, OUTPUT);
  digitalWrite(IO09, LOW);
  digitalWrite(IO13, HIGH);
}

void loop() {
  digitalWrite(IO09, HIGH);
  delay(ON_MS);
  digitalWrite(IO09, LOW);
  delay(OFF_MS);
  digitalWrite(IO13, HIGH);
  delay(ON_MS);
  digitalWrite(IO13, LOW);
  delay(OFF_MS);
}