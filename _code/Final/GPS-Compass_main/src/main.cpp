#include <Arduino.h>

#include "GPIO_MAP.h"
#include "CHARGER.h"
#include "SHIFT_REG.h"
#include "DIST_CALC.h"
#include "WEBSERVER.h"
#define DIGIT_COUNT 3
#include "7SEG.h"


Bq25895m charger;
//Display segDisplay;

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Setup started.");
  // Charger setup
  charger.setup();
  web_setup();


  display_setup();
  //segDisplay.setup(MOSI_SPI, CLK_SPI, CS_SPI);
  //segDisplay.printNum(123, 2, false);
  //segDisplay.setDecimalP(1, true);
  //segDisplay.flush();


  // shift register setup
  mask = 0ULL;
  shift_8byte(mask);
  triggerBuffers();

  Serial.println("Setup completed.");
}

void loop() {
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
  if (northAngle >= 360.0) northAngle -= 360.0;

  int northLed;
  unsigned long long northMask = angleToLedMask(northAngle, northLed);

  // Toggle north LED state every NORTH_BLINK_INTERVAL
  unsigned long now = millis();
  if (now - lastNorthToggle >= NORTH_BLINK_INTERVAL) {
    lastNorthToggle = now;
    northLedState = !northLedState;
  }
  unsigned long long displayedNorthMask = northLedState ? northMask : 0ULL;

  Serial.println();
  Serial.print("Heading [deg]: ");
  Serial.println(heading, 2);

  Serial.print("North angle [deg]: ");
  Serial.println(northAngle, 2);

  Serial.print("North LED index: ");
  Serial.println(northLed);

  Serial.print("North LED mask: ");
  printMask(displayedNorthMask);

  // -------- TARGET --------
  double bearing = bearingToTarget(compass, target);
  double relativeAngle = bearing - heading;
  if (relativeAngle < 0) relativeAngle += 360.0;

  int targetLed;
  unsigned long long targetMask = angleToLedMask(relativeAngle, targetLed);

  Serial.println();
  Serial.print("Bearing to target [deg]: ");
  Serial.println(bearing, 2);

  Serial.print("Relative angle to target [deg]: ");
  Serial.println(relativeAngle, 2);

  Serial.print("Target LED index: ");
  Serial.println(targetLed);

  Serial.print("Target LED mask: ");
  printMask(targetMask);

  // Combine masks (display both north and target). Use displayedNorthMask to blink north.
  mask = displayedNorthMask | targetMask;

  // send mask to shift register
  shift_8byte(mask);
  triggerBuffers();

  int distance = round(distance_calc(compass, target));



  int distance_kilo = 0; // Convert to kilometers.
  bool dp0, dp1, dp2;
  dp0 = dp1 = dp2 = false;

  if(distance > 999 && distance < 10000) { //eg: 1000m = 1.0km
    distance_kilo = distance / 10; // Convert to kilometers.
    dp2 = true;
  } else if(distance >  9999 && distance < 100000) { //eg: 10000m = 10.0km
    distance_kilo = distance / 100; // Convert to kilometers.
    dp1 = true;
  } else if(distance > 99999) {
    distance_kilo = distance / 1000; // Convert to kilometers.
    dp0 = true;
  } else {
  }
    displayDigit(2, distance_kilo % 10, dp0);
    displayDigit(1, distance_kilo % 100 / 10, dp1);
    displayDigit(0, distance_kilo / 100, dp2);
    push();

  //1000m   = 1.,0,0
  //10000m  = 1,0.,0
  //100000m = 1,0,0.
  

  // wait so serial output is not constant
  delay(100);
  // heading += 2;
}