#include <Arduino.h>

#include "7SEG.h"
#include "CHARGER.h"
#include "DIST_CALC.h"
#include "GNSS.h"
#include "GPIO_MAP.h"
//#include "IMU.h"
#include "SHARED_TYPES.h"
#include "SHIFT_REG.h"
#include "WEBSERVER.h"





Bq25895m charger;





void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Setup started.");

  display_setup();  // 7-segment display setup
  charger.setup();  // Charger setup
  gnss_setup();     // GNSS module setup
  //imu_setup();    // IMU setup
  web_setup();      // Webserver setup

  unsigned long long mask = 0ULL;
  shift_8byte(mask);
  triggerBuffers();

  Serial.println("Setup completed.");
}









void loop() {

  unsigned long long mask = 0ULL;
  // Recompute masks and print values periodically

  static Position target;
  web_ret webRet;
  webRet = web_loop();
  if(webRet.err == webRetErr::OKI) {
    target.lat = webRet.pos.lat;
    target.lon = webRet.pos.lon;
  }
  
  // Position of the compass.
  // Update it from the GNSS module.
  static Position compass;

  // Distance
  Serial.println();
  Serial.print("Distance [m]: ");
  Serial.println(distance_calc(compass, target), 2);

  // -------- NORTH --------
  double northAngle = 360.0 - heading;

  unsigned long long northMask = angleToLedMask(northAngle);

  // Toggle north LED state every NORTH_BLINK_INTERVAL


  // -------- TARGET --------
  double bearing = bearingToTarget(compass, target);
  double relativeAngle = bearing - heading;

  
  unsigned long long targetMask = angleToLedMask(relativeAngle);



  // Combine masks (display both north and target). Use displayedNorthMask to blink north.


  unsigned long now = millis();
  if (now - lastNorthToggle >= NORTH_BLINK_INTERVAL) {
    lastNorthToggle = now;
    northLedState = !northLedState;
  }
  if (northLedState) {
    mask = mask | northMask;
  }
  mask = mask | targetMask;

  // send mask to shift register
  shift_8byte(mask);
  triggerBuffers();

  int distance = round(distance_calc(compass, target));

  // wait so serial output is not constant
  delay(100);
  // heading += 2;
}