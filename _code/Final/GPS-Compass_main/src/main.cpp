#include <Arduino.h>
#include <ctype.h> 

#include "SYSTEM_STATE.h"
#include "7SEG.h"
#include "CHARGER.h"
#include "GNSS_NMEA.h"
#include "WEBSERVER.h"
#include "GPIO_MAP.h"
#include "DIST_CALC.h"
#include "IMU.h"
#include "SHIFT_REG.h"
#include "LED_MASK.h"

HardwareData sys; 
Position target_position;
MonitorState mon;

unsigned long lastNorthToggle = 0;
const unsigned long NORTH_BLINK_INTERVAL = 500;
bool northLedState = true;
bool lastButtonState = HIGH; 

void printSerialMenu() {
  Serial.println("\n========================================");
  Serial.println("       COMMAND HELPER (SYSTEM READY)    ");
  Serial.println("========================================");
  Serial.println(" [m] -> System Monitor (Battery, Current)");
  Serial.println(" [d] -> Navigation (Distance, Heading, Bearing)");
  Serial.println(" [p] -> Position (Current Lat/Lon, Target)");
  Serial.println(" [e] -> Exit / Stop active monitor");
  Serial.println("========================================\n");
}

void runPeriodicMonitor() {

    if (mon.mode == 'e') return;
    unsigned long now = millis();

    if (now - mon.lastPrint >= mon.INTERVAL) {
        mon.lastPrint = now;
        Serial.printf("\n--- [%c] Report ---\n", toupper(mon.mode));
        if (mon.mode == 'm') {
            Serial.printf("Battery: %.2fV | Current: %.2fmA | Temp: %.1fC\n", 
                          sys.vbat, sys.ichg, sys.temp);
        } 
        else if (mon.mode == 'd') {
            Serial.printf("Dist: %dm | Head: %.1f | Bear: %.1f\n", 
                          (int)distance_to_target(), imu_getHeading(), bearing_to_target());
        } 
        else if (mon.mode == 'p') {
            Serial.printf("Current: Lat %.6f, Lon %.6f\n", compass_lat, compass_lon);
            Serial.printf("Target:  Lat %.6f, Lon %.6f\n", target_position.lat, target_position.lon);
        }
        Serial.println("------------------");
    }
}

void handleSerialCommands() {
  if (Serial.available() > 0) {
    char incomingByte = Serial.read();
    if (incomingByte == '\n' || incomingByte == '\r') return;

    switch (incomingByte) {
      case 'm': case 'd': case 'p':
        mon.mode = incomingByte; 
        Serial.printf(">> Mode Set: [%c]\n", mon.mode);
        mon.lastPrint = millis() - mon.INTERVAL; 
        break;

      case 'e':
        mon.mode = 'e';
        Serial.println(">> Monitor Stopped.");
        break;
        
      case '?': case 'h':
        printSerialMenu();
        break;
    }
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_I2C, SCL_I2C, 100000);
  
  pinMode(CLK_SR, OUTPUT);
  pinMode(SI_SR, OUTPUT);
  pinMode(LATCH_SR, OUTPUT);
  pinMode(DBNCD_0, INPUT_PULLUP);
  pinMode(DBNCD_1, OUTPUT);

  display_setup();
  charger_setup();
  charger_setCurrentRaw(CURR_1A); 
  web_setup(); 
  gnss_setup();
  imu_setup();

  digitalWrite(DBNCD_1, LOW); 
  printSerialMenu();
}

void loop() {

  handleSerialCommands();

  bool currentButtonState = digitalRead(DBNCD_0);

  if (lastButtonState == HIGH && currentButtonState == LOW) {
    sys.webServerEnabled = !sys.webServerEnabled;
    digitalWrite(DBNCD_1, sys.webServerEnabled ? HIGH : LOW);
    Serial.printf("\nSystem: Webserver %s\n", sys.webServerEnabled ? "ON" : "OFF");
    delay(50); 
  }
  lastButtonState = currentButtonState;

  // --- HARDWARE UPDATES ---
  sys.vbat = charger_readBatteryVoltage();
  sys.ichg = charger_readChargeCurrent();
  sys.temp = charger_readBatteryTemp();

  if (sys.webServerEnabled) web_loop(); 

  gnss_update();               
  gnss_checkSerialBridge();    

  // --- Navigation and display ---
  float heading = imu_getHeading();
  double bearing = bearing_to_target(); 
  int distance = (int)round(distance_to_target()); 
  
  convertDistanceToBuffer(distance);
  push();

  // --- Blink Logic ---
  if (millis() - lastNorthToggle >= NORTH_BLINK_INTERVAL) {
    lastNorthToggle = millis();
    northLedState = !northLedState;
  }

  uint64_t finalMask = getNorthMask(heading, northLedState) | getTargetMask(heading, (float)bearing);
  shift_8byte(finalMask);
  triggerBuffers();

  runPeriodicMonitor();
  delay(20);
}