#include <Arduino.h>
#include "7SEG.h"
#include "CHARGER.h"
#include "GNSS_NMEA.h"
#include "WEBSERVER.h"
#include "GPIO_MAP.h"
#include "DIST_CALC.h"

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
  gnss_setup();

  Serial.println("Setup complete");

}


void loop() {

  g_vbat = charger_readBatteryVoltage();
  g_ichg = charger_readChargeCurrent();

  web_loop();        // updates target_position
  /* ================= RAW + PARSE ================= */

while (GNSS.available()) {
    char c = GNSS.read();

    Serial.write(c);            // RAW output
    gnss_parser.encode(c);      // Parse RMC/GGA
    gnss_parse_gsv(c);          // Parse GSV
}

  /* ================= NAVIGATION ================= */
  float distance_float = distance_to_target();     // from DIST_CALC
  int distance = round(distance_float);
  double bearing = bearing_to_target();      // from DIST_CALC

    /* ================= SERIAL OUTPUT ================= */

  Serial.println("---- NAV DATA ----");

  Serial.print("Current Lat: ");
  Serial.println(compass_lat, 6);

  Serial.print("Current Lon: ");
  Serial.println(compass_lon, 6);

  Serial.print("Target Lat: ");
  Serial.println(target_position.lat, 6);

  Serial.print("Target Lon: ");
  Serial.println(target_position.lon, 6);

  Serial.print("Distance (m): ");
  Serial.println(distance_float);

  Serial.print("Bearing (deg): ");
  Serial.println(bearing);

  Serial.print("Satellites Used (GGA): ");
  Serial.println(compass_satellites_used);

  Serial.print("Satellites In View (GSV): ");
  Serial.println(compass_satellites_in_view);

  Serial.println("------------------");
  delay(1000);  // Update every second
  Serial.print("Available: ");
  Serial.println(GNSS.available());


  /* ================= DISPLAY ================= */

  convertDistanceToBuffer(distance);
  push();

}

