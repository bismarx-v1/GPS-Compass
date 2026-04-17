#include <Arduino.h>
#include <ctype.h> 
#include "esp_sleep.h"

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
#include "EEPROM.h"

// --- Global Variables ---
HardwareData sys; 
Position target_position;
MonitorState mon;
UIState ui; 

// --- Helper Functions ---
void printSerialMenu() {
  Serial.println("\n========================================");
  Serial.println("        COMMAND HELPER (SYSTEM READY)   ");
  Serial.println("========================================");
  Serial.println(" [m] -> System Monitor (Battery, Current)");
  Serial.println(" [d] -> Navigation (Distance, Heading, Bearing)");
  Serial.println(" [p] -> Position (Current Lat/Lon, Target)");
  Serial.println(" [w] -> Webserver Status");
  Serial.println(" [g] -> GNSS Monitor (Fix, Satellites)");
  Serial.println(" [i] -> IMU Monitor (Heading, Calibration)");
  Serial.println(" [s] -> Sleep Mode (Light Sleep)");
  Serial.println(" [c] -> Save IMU Calibration to EEPROM");
  Serial.println(" [e] -> Exit / Stop active monitor");
  Serial.println(" [?] or [h] -> Print this menu");
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
            Serial.printf("Current: Lat %.6f, Lon %.6f\n", gnss_getLat(), gnss_getLon());
            Serial.printf("Target:  Lat %.6f, Lon %.6f\n", target_position.lat, target_position.lon);
        }
        else if (mon.mode == 'w') {
            Serial.printf("Webserver Status: %s\n", sys.webServerEnabled ? "ON" : "OFF\n");
            Serial.print("IP: ");
            Serial.println(WiFi.softAPIP());
        }
        else if (mon.mode == 'g') {
            gnss_monitor(); 
        }
        else if (mon.mode == 'i') {
            imu_monitor();
        }
        else if (mon.mode == 'c') {
            Serial.println("Save Calibration.");
            //imu_saveCalibration();
        }
        else {
            Serial.println("Unknown mode. Press [?] or [h] for help.");
        }
        Serial.println("------------------");
    }
}
 

void enterLightSleep() {
  Serial.println("\nEntering Light Sleep...");
  
  // Turn off LED during sleep to save power
  digitalWrite(DBNCD_1, LOW); 

  clearSpi();
  shift_8byte(0ULL);
  triggerBuffers();
  
  // Configure Wakeup on GPIO 35 (DBNCD_0)
  gpio_wakeup_enable((gpio_num_t)DBNCD_0, GPIO_INTR_LOW_LEVEL);
  esp_sleep_enable_gpio_wakeup();

  esp_light_sleep_start();

  // --- WAKE UP ---
  // Immediately restore LED state
  digitalWrite(DBNCD_1, sys.webServerEnabled ? HIGH : LOW);
  
  // Force state update to prevent immediate toggle upon wake
  sys.lastButtonState = digitalRead(DBNCD_0); 
  
  Serial.println("System Awake.");
}

void handleSerialCommands() {
  if (Serial.available() > 0) {
    char incomingByte = Serial.read();
    if (incomingByte == '\n' || incomingByte == '\r') return;

    switch (incomingByte) {
      case 'm': case 'd': case 'p': case 'w': case 'g': case 'i': case 'c':
        mon.mode = incomingByte; 
        Serial.printf(">> Mode Set: [%c]\n", mon.mode);
        mon.lastPrint = millis() - mon.INTERVAL; 
        break;

      case 's':
        enterLightSleep();
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

// --- Main Program ---
void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_I2C, SCL_I2C, 100000);
  
  // Read saved coordinates from EEPROM after I2C bus is initialized
  eepromRead(0x0000, target_position.lat);
  eepromRead(0x0004, target_position.lon);
  
  // Sanity check for uninitialized EEPROM (prevents NaN errors)
  if (isnan(target_position.lat)) target_position.lat = 0.0f;
  if (isnan(target_position.lon)) target_position.lon = 0.0f;
  // -----------------------
  
  // Pin Configuration
  pinMode(CLK_SR, OUTPUT);
  pinMode(SI_SR, OUTPUT);
  pinMode(LATCH_SR, OUTPUT);

  pinMode(DBNCD_0, INPUT);
  pinMode(DBNCD_1, OUTPUT); 
  digitalWrite(DBNCD_1, LOW); 
  pinMode(DBNCD_2, OUTPUT);
  digitalWrite(DBNCD_2, LOW);
  // Hardware Initialization
  display_setup();
  charger_setup();
  charger_setCurrentRaw(CURR_1A);
  gnss_setup();
  imu_setup(); 
  printSerialMenu();
}

void loop() {
  // 1. Process Inputs
  handleSerialCommands();

  bool currentButtonState = digitalRead(DBNCD_0);

  digitalWrite(DBNCD_2, gnss_hasFix() ? HIGH : LOW);

  if (sys.lastButtonState == HIGH && currentButtonState == LOW) {
    sys.webServerEnabled = !sys.webServerEnabled;
    
    if (sys.webServerEnabled) {
        // Turn ON
        web_setup(true);   
        digitalWrite(DBNCD_1, HIGH);
        Serial.println(">> Access Point: ENABLED");
    } else {
        // Turn OFF
        WiFi.softAPdisconnect(true); // Disconnect clients and close AP
        WiFi.mode(WIFI_OFF);         // Power down the radio hardware
        digitalWrite(DBNCD_1, LOW);
        Serial.println(">> Access Point: DISABLED (Radio Off)");
    }
}
sys.lastButtonState = currentButtonState;

  // 2. Hardware Updates
  sys.vbat = charger_readBatteryVoltage();
  sys.ichg = charger_readChargeCurrent();
  sys.temp = charger_readBatteryTemp();

  if (sys.webServerEnabled) {
      web_loop(); 
  }
  
  gnss_update();               
  gnss_checkSerialBridge();    

 // 3. Navigation Calculations & GNSS Check
  bool hasFix = gnss_hasFix();
  float heading = imu_getHeading();
  bool northState = (millis() % 1000 < 500); // 1Hz Blink

  if (!hasFix) {
    
    segmentsArrayObjGlobal[0] = 0b00000001; // '-' (Segment G)
    segmentsArrayObjGlobal[1] = 0b00000001; // '-' (Segment G)
    segmentsArrayObjGlobal[2] = 0b00000001; // '-' (Segment G)
    push();

  } else {
      int distance = (int)round(distance_to_target()); 
      convertDistanceToBuffer(distance);
      push();
  }

  uint64_t finalMask = 0ULL;

  if (triggerBlink) {
      // Handle Blink Timing
      if (millis() - lastBlinkStep >= BLINK_DURATION) {
          lastBlinkStep = millis();
          blinkStep--;
          if (blinkStep <= 0) triggerBlink = false;
      }
      finalMask = (blinkStep % 2 != 0) ? getCheckerboardMask() : 0ULL;
  } 
  else if (!hasFix) {
      // No GNSS Fix: Only show North LED (Heading)
      finalMask = getNorthMask(heading, northState);
  }
  else {
      // Normal Navigation Logic: North + Target Bearing
      finalMask = getNorthMask(heading, northState) | 
                  getTargetMask(heading, bearing_to_target());
  }

  shift_8byte(finalMask);
  triggerBuffers();

  // 5. Outputs
  runPeriodicMonitor();
  delay(20);
}