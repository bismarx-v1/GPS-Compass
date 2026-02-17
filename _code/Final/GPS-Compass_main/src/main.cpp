#include <Arduino.h>
#include "7SEG.h"
#include "CHARGER.h"
#include "WEBSERVER.h"
#include "GPIO_MAP.h"

float distance_float = 123.0;
int distance = round(distance_float);

float g_vbat = 0;
float g_ichg = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Setup initialized");

  display_setup();

  charger_setup();
  charger_setCurrentRaw(CURR_1A);  // Sets charging current, Available options: CURR_0_5A, CURR_1A, CURR_1_5A, CURR_2A

  web_setup();

  Serial.println("Setup complete");

}


void loop() {

  g_vbat = charger_readBatteryVoltage();
  g_ichg = charger_readChargeCurrent();
  
  web_loop();
  convertDistanceToBuffer(distance);
  push();
  //charger_monitor();
  
  uint8_t raw;
  regGet(REG0E, raw);
  Serial.printf("0x%02x (%u) (0%o)\n", raw, raw, raw);
  delay(500);
}
