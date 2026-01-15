#include <Arduino.h>

#include "GPIO_MAP.h"
#include "CHARGER.h"

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Setup started.");

  Bq25895m charger;
  charger.setup();

  Serial.println("Setup completed.");
}

void loop() {
    // put your main code here, to run repeatedly:
}