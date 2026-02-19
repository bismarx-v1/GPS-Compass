#ifndef TESEO_GNSS_H
#define TESEO_GNSS_H

#include <Arduino.h>
#include <Wire.h>
#include <TinyGPS++.h>
#include "GPIO_MAP.h"
// #include <GPIO_MAP.h> // Uncomment if you have your pins defined here

// ==========================
// Hardware Definitions
// ==========================
#define I2C_GNSS_ADDR   0x3A  
#define REG_NMEA_STREAM 0xFF  
#define TESEO_DUMMY     0xFF  

// If not defined in GPIO_MAP.h, define them here for ESP32-S3


/* * REFERENCE: CONFIGURATION COMMANDS (Permanent/Parametric)
 * --------------------------------------------------------
 * $PSTMSETPAR      : Set System Parameter in the configuration data block
 * $PSTMGETPAR      : Get System Parameter from configuration data block
 * $PSTMSAVEPAR     : Save System Parameters in the GNSS backup memory
 * $PSTMRESTOREPAR  : Restore System Parameters (Factory Settings)
 * $PSTMCFGPORT     : Char Port Configuration
 * $PSTMCFGMSGL     : Message List Configuration
 * $PSTMCFGGNSS     : GNSS Algorithm Configuration
 * $PSTMCFGSBAS     : SBAS Algorithm Configuration
 * $PSTMCFGPPSGEN   : PPS General Configuration
 * $PSTMCFGPPSSAT   : PPS Satellite Related Configuration
 * $PSTMCFGPPSPUL   : PPS Pulse Related Configuration
 * $PSTMCFGPOSHOLD  : Configure the Position hold
 * $PSTMCFGTRAIM    : Traim Configuration
 * $PSTMCFGSATCOMP  : Configure the PPS with general settings
 * $PSTMCFGLPA      : Configure the Low Power Algorithm
 * $PSTMCFGAGPS     : Assisted GNSS Configuration
 * $PSTMCFGAJM      : Anti-Jamming Configuration
 * $PSTMCFGODO      : Odometer Configuration
 * $PSTMCFGLOG      : Logger Configuration
 * $PSTMCFGGEOFENCE : Geofencing Configuration
 * $PSTMCFGGEOCIR   : Geofencing Circle Configuration
 * $PSTMCFGCONST    : Allow enable/disable all the GNSS constellations
 * * REFERENCE: GNSS MANAGEMENT COMMANDS (Executables/Actions)
 * --------------------------------------------------------
 * $PSTMINITGPS           : Initialize GPS position and time
 * $PSTMINITTIME          : Initialize time only
 * $PSTMINITFRQ           : Initialize center frequency
 * $PSTMSETRANGE          : Set frequency range for satellite searching
 * $PSTMCLREPHS           : Clear all ephemeris
 * $PSTMDUMPEPHEMS        : Dump Ephemeris data
 * $PSTMEPHEM             : Load Ephemeris data
 * $PSTMCLRALMS           : Clear all almanacs
 * $PSTMDUMPALMANAC       : Dump Almanacs data
 * $PSTMALMANAC           : Load Almanacs data
 * $PSTMCOLD              : Perform COLD start
 * $PSTMWARM              : Perform WARM start
 * $PSTMHOT               : Perform HOT start
 * $PSTMSRR               : System Reset (Software Reboot)
 * $PSTMGPSRESET          : Reset the GPS engine
 * $PSTMGPSSUSPEND        : Suspend GPS engine
 * $PSTMGPSRESTART        : Restart GPS engine
 * $PSTMGNSSINV           : Invalidate the GNSS fix status
 * $PSTMTIMEINV           : Invalidate the GPS time
 * $PSTMGETSWVER          : Provide the GPS library version string
 * $PSTMSBASONOFF         : Enable/Disable the SBAS activity
 * $PSTMSBASSERVICE       : Set the SBAS service
 * $PSTMSBASSAT           : Set the SBAS satellite's ID
 * $PSTMSBASM             : Send a SBAS frame
 * $PSTMRFTESTON          : Enable the RF test mode
 * $PSTMRFTESTOFF         : Disable the RF test mode
 * $PSTMGETALGO           : Get FDE algorithm ON/OFF status
 * $PSTMSETALGO           : Set FDE algorithm ON/OFF status
 * $PSTMGETRTCTIME        : Get the current RTC time
 * $PSTMDATUMSELECT       : Set a geodetic local datum different from WGS84
 * $PSTMDATUMSETPARAM     : Set parameters for datum transformations
 * $PSTMENABLEPOSITIONHOLD: Set status and position for Position Hold
 * $PSTMSETCONSTMASK      : Set GNSS constellation mask
 * $PSTMNOTCH             : Set the ANF operation mode
 * $PSTMLOWPOWERONOFF     : Set low power algorithm parameters
 * $PSTMNMEAREQUEST       : Send NMEA messages according to input list
 * $PSTMFORCESTANDBY      : Force platform to standby mode
 * $PSTMIONOPARAMS        : Upload iono packet into NVM
 * $PSTMGALILEOGGTO       : Program Galileo broadcast GGTO
 * $PSTMGALILEODUMPGGTO   : Dump broadcast GGTO
 * $PSTMSETTHTRK          : Configure CN0/Elevation for tracking
 * $PSTMSETTHPOS          : Configure CN0/Elevation for positioning
 */

// ==========================
// Global Parser Object
// ==========================
// The TinyGPS++ object that will parse the NMEA data
static TinyGPSPlus teseoGPS;


// ==========================
// Low Level I2C / Core Functions
// ==========================

inline uint8_t gnss_calculateChecksum(const char* str) 
{
    uint8_t checksum = 0;
    while (*str) checksum ^= *str++;
    return checksum;
}

inline void gnss_sendRaw(const char* msg) 
{
    Wire.beginTransmission(I2C_GNSS_ADDR);
    Wire.write(REG_NMEA_STREAM); 
    Wire.write((uint8_t*)msg, strlen(msg));
    
    // Ensure standard NMEA termination
    int len = strlen(msg);
    if (len > 0 && msg[len-1] != '\n') {
        Wire.write('\r');
        Wire.write('\n');
    }
    
    uint8_t error = Wire.endTransmission();
    if (error != 0) {
        Serial.print("[GNSS I2C Write Error: "); Serial.print(error); Serial.println("]");
    }
}

inline void gnss_executeCommand(const char* cmdBody) 
{
    char fullMsg[128];
    uint8_t cs = gnss_calculateChecksum(cmdBody);
    sprintf(fullMsg, "$%s*%02X\r\n", cmdBody, cs);
    gnss_sendRaw(fullMsg);
}


// ==========================
// Read & Parse Logic
// ==========================

// Call this continuously in your main loop() to keep the parser fed
inline void gnss_update() 
{
    // Request a chunk of bytes from the GNSS I2C buffer
    uint8_t bytesReceived = Wire.requestFrom(I2C_GNSS_ADDR, 32);
    
    if (bytesReceived > 0) {
        while (Wire.available()) {
            uint8_t c = Wire.read();
            // Filter out dummy/empty bytes
            if (c != TESEO_DUMMY && c != 0x00) {
                // Feed the valid character to the parser
                teseoGPS.encode(c);
                
                // Optional: Uncomment below to print raw NMEA strings to Serial
                // Serial.write((char)c); 
            }
        }
    }
}

// ==========================
// Serial Bridge (Optional)
// ==========================

inline void gnss_checkSerialBridge() 
{
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        input.trim();

        if (input.length() > 0) {
            if (input.startsWith("$")) {
                gnss_sendRaw(input.c_str());
            } else {
                gnss_executeCommand(input.c_str());
            }
        }
    }
}

// ==========================
// High Level Parsed Getters
// ==========================

inline bool gnss_hasFix() {
    // Valid location means the module has a 2D or 3D fix
    return teseoGPS.location.isValid();
}

inline double gnss_getLatitude() {
    return teseoGPS.location.lat();
}

inline double gnss_getLongitude() {
    return teseoGPS.location.lng();
}

inline double gnss_getAltitude() {
    return teseoGPS.altitude.meters();
}

inline uint32_t gnss_getSatellites() {
    return teseoGPS.satellites.value();
}

inline double gnss_getSpeedKmph() {
    return teseoGPS.speed.kmph();
}

// ==========================
// Setup & Monitor Functions
// ==========================

inline void gnss_setup() 
{
    // Assuming Wire.begin() might be called in your main or charger setup.
    // If not, it's safe to call it multiple times, but let's ensure it's running.
    Wire.begin(SDA_I2C, SCL_I2C);
    Wire.setClock(400000); // 400kHz fast mode

    delay(500); // Give module time to wake up

    // Apply standard baseline configurations
    gnss_executeCommand("PSTMSETCONSTMASK,15"); // Use all satellites
    gnss_executeCommand("PSTMSETPAR,1303,1.0"); // 1Hz fix rate
}

inline void gnss_monitor()
{
    if (gnss_hasFix()) 
    {
        Serial.print("GNSS Fix -> Lat: ");
        Serial.print(gnss_getLatitude(), 6);
        Serial.print(" | Lon: ");
        Serial.print(gnss_getLongitude(), 6);
        Serial.print(" | Alt: ");
        Serial.print(gnss_getAltitude());
        Serial.print("m | Sats: ");
        Serial.println(gnss_getSatellites());
    } 
    else 
    {
        Serial.print("Searching for satellites... Found: ");
        Serial.println(gnss_getSatellites());
    }
}

#endif // TESEO_GNSS_H