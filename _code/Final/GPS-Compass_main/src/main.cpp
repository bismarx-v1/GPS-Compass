#include <Arduino.h>
#include "7SEG.h"
#include "CHARGER.h"
#include "GNSS_NMEA.h"
#include "WEBSERVER.h"
#include "GPIO_MAP.h"
#include "DIST_CALC.h"

float g_vbat = 0;
float g_ichg = 0;

unsigned long lastPrint = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Setup initialized");

  display_setup();
  charger_setup();
  charger_setCurrentRaw(CURR_1A);  // Sets charging current, Available options: CURR_0_5A, CURR_1A, CURR_1_5A, CURR_2A
  web_setup();
  gnss_setup();

  Serial.println("Setup complete");

}

void loop() {

  g_vbat = charger_readBatteryVoltage();
  g_ichg = charger_readChargeCurrent();

  web_loop();                 // updates target_position
  gnss_update();              // updates teseoGPS object with latest NMEA data
  gnss_checkSerialBridge();   // allows sending commands via Serial Monitor

  /* ================= NAVIGATION ================= */
  float distance_float = distance_to_target();     
  int distance = round(distance_float);
  double bearing = bearing_to_target();           
  
  /* ================= DISPLAY ================= */
  convertDistanceToBuffer(distance);
  push();

    if (millis() - lastPrint > 2000) {
        lastPrint = millis();
        
        Serial.println("--- System Status ---");
        charger_monitor();
        gnss_monitor(); 
        Serial.print("Distance: ");
        Serial.print(distance);
        Serial.print("m, Bearing: ");
        Serial.print(bearing);
        Serial.println("deg");

        // Example of accessing the variables directly in main:
        if (gnss_hasFix()) {
            double compass_lat = gnss_getLatitude();
            double compass_lon = gnss_getLongitude();
            Serial.print("Current Position: ");
            Serial.print(compass_lat, 6);
            Serial.print(", ");
            Serial.println(compass_lon, 6);
        }
    }

}

