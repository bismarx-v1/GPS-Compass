#include <Arduino.h>
#include "7SEG.h"
#include "GPIO_MAP.h"

float distance_float = 123.0;
int distance = round(distance_float);

void setup() {
  Serial.begin(115200);
  Serial.println("Setup initialized");
  display_setup();
  
  Serial.println("Setup complete");

}

void loop() { 
  convertDistanceToBuffer(distance);
  push();
}