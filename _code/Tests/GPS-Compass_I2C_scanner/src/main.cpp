#include <Arduino.h>
#include <Wire.h>

// ==========================
// Hardware & Register Definitions
// ==========================
#define I2C_GNSS_ADDR   0x3A  
#define SDA_PIN         6     
#define SCL_PIN         7     
#define REG_NMEA_STREAM 0xFF  
#define TESEO_DUMMY     0xFF  

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
 * 
 * 
 * REFERENCE: GNSS MANAGEMENT COMMANDS (Executables/Actions)
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
// Communication Functions
// ==========================

uint8_t calculateChecksum(const char* str) {
    uint8_t checksum = 0;
    while (*str) checksum ^= *str++;
    return checksum;
}

// Low-level write to Register 0xFF
void gnss_sendRaw(const char* msg) {
    Wire.beginTransmission(I2C_GNSS_ADDR);
    Wire.write(REG_NMEA_STREAM); 
    Wire.write((uint8_t*)msg, strlen(msg));
    
    // Ensure standard NMEA termination
    if (msg[strlen(msg)-1] != '\n') {
        Wire.write('\r');
        Wire.write('\n');
    }
    
    uint8_t error = Wire.endTransmission();
    if (error != 0) {
        Serial.print("[I2C Error: "); Serial.print(error); Serial.println("]");
    }
}

// High-level: adds $, *, CS, and line endings
void gnss_executeCommand(const char* cmdBody) {
    char fullMsg[128];
    uint8_t cs = calculateChecksum(cmdBody);
    sprintf(fullMsg, "$%s*%02X\r\n", cmdBody, cs);
    gnss_sendRaw(fullMsg);
}

// ==========================
// Bridge & Stream Logic
// ==========================

void checkSerialBridge() {
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        input.trim();

        if (input.length() > 0) {
            // If it's a full pasted command with $...*CS
            if (input.startsWith("$")) {
                Serial.print("Bridge -> Sending Raw: ");
                Serial.println(input);
                gnss_sendRaw(input.c_str());
            } 
            // If it's just the command text (e.g. PSTMSRR)
            else {
                Serial.print("Bridge -> Executing: ");
                Serial.println(input);
                gnss_executeCommand(input.c_str());
            }
        }
    }
}

void gnss_readStream() {
    uint8_t bytesReceived = Wire.requestFrom(I2C_GNSS_ADDR, 32);
    if (bytesReceived > 0) {
        while (Wire.available()) {
            uint8_t c = Wire.read();
            if (c != TESEO_DUMMY && c != 0x00) {
                Serial.write((char)c);
            }
        }
    }
}

// ==========================
// Setup and Main Loop
// ==========================

void setup() {
    Serial.begin(115200);
    while (!Serial);

    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(400000);
    Wire.setTimeOut(100);

    delay(1000); // Allow some time for the GNSS module to power up
    Serial.println("\n--- Teseo-LIV3R System Initializing ---");

    // --- STARTUP CONFIGURATION ---
    // These run once at boot to ensure the module is in the correct state.
    Serial.println("Applying Startup Config...");
    
    //gnss_executeCommand("PSTMSETCONSTMASK,15"); // Use all satellites
    //gnss_executeCommand("PSTMSETPAR,1303,1.0"); // 1Hz fix rate
    
    // Optional: Uncomment below to make these settings survive a power cycle
    // gnss_executeCommand("PSTMSAVEPAR"); 
    // gnss_executeCommand("PSTMSRR"); 
    // delay(2000); 

    Serial.println("Startup Config applied. Serial Bridge Active.");
    Serial.println("Usage: Paste full commands starting with $ or type command names like PSTMSRR.\n");
}

void loop() {
    // 1. Check for manual commands from Serial Monitor
    checkSerialBridge();

    // 2. Continually read the NMEA stream
    gnss_readStream();

    delay(5);
}