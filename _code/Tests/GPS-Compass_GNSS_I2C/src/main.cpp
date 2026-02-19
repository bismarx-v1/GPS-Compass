#include <Arduino.h>
#include <Wire.h>

// ==========================
// CONFIGURATION
// ==========================
#define I2C_GNSS_ADDR   0x3A  
#define REG_NMEA_STREAM 0xFF
#define TESEO_DUMMY     0xFF

// Use -1 to tell the ESP32 to use its "Default" hardware pins
// For most ESP32s, this is SDA=21, SCL=22. For S3/C3, it varies.
#define SDA_PIN -1 
#define SCL_PIN -1 

// ==========================
// TOOL: I2C SCANNER
// ==========================
// This helps verify if the module is actually "alive" on the bus
void scanI2CBus() {
    Serial.println("\nScanning I2C Bus...");
    byte error, address;
    int nDevices = 0;

    for (address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0) {
            Serial.print("Device found at address 0x");
            if (address < 16) Serial.print("0");
            Serial.print(address, HEX);
            if (address == I2C_GNSS_ADDR) Serial.println(" (Teseo GNSS!)");
            else Serial.println();
            nDevices++;
        }
    }
    if (nDevices == 0) Serial.println("No I2C devices found. CHECK WIRING!");
    else Serial.println("Scan complete.\n");
}

// ==========================
// GNSS FUNCTIONS
// ==========================

void gnss_sendCommand(const char* rawCommand) {
    char fullBuffer[128];
    
    // Calculate NMEA Checksum (XOR)
    uint8_t checksum = 0;
    for (int i = 0; i < strlen(rawCommand); i++) {
        checksum ^= rawCommand[i];
    }

    // Format the command: $CMD*CS\r\n
    sprintf(fullBuffer, "$%s*%02X\r\n", rawCommand, checksum);

    Serial.print("Sending: "); Serial.print(fullBuffer);

    Wire.beginTransmission(I2C_GNSS_ADDR);
    Wire.write(REG_NMEA_STREAM); // Register index 0xFF [Section 2.3.2.3]
    Wire.write((const uint8_t*)fullBuffer, strlen(fullBuffer));
    
    uint8_t error = Wire.endTransmission();
    
    if (error != 0) {
        Serial.print("Transmission Error: "); Serial.println(error);
    } else {
        Serial.println("Sent successfully.");
    }
}

void gnss_readStream() {
    // Request data from stream register [Section 2.3.2.1]
    uint8_t count = Wire.requestFrom(I2C_GNSS_ADDR, 32);
    
    while (Wire.available()) {
        uint8_t c = Wire.read();
        // Skip 0xFF (Idle) and 0x00 (Padding)
        if (c != TESEO_DUMMY && c != 0x00) {
            Serial.write((char)c);
        }
    }
}

// ==========================
// MAIN SETUP & LOOP
// ==========================

void setup() {
    Serial.begin(115200);
    while (!Serial);
    delay(1000); 

    Serial.println("--- Teseo-LIV3R Debug Initialized ---");

    // Initialize I2C using default pins
    // If you are on a custom board, replace -1 with your specific Pin numbers
    if (!Wire.begin(SDA_PIN, SCL_PIN)) {
        Serial.println("CRITICAL: Wire.begin failed! Check if pins are valid.");
        while (1); // Stop here if I2C fails
    }

    // Run the scanner to confirm hardware connection
    scanI2CBus();

    // Configuration Commands
    // PSTMSETPAR,1303,1.0 sets the fix rate to 1Hz [Table 159]
    gnss_sendCommand("PSTMSETPAR,1303,1.0");
    
    // PSTMSRR performs a System Reset to apply changes [Section 10.2.1]
    gnss_sendCommand("PSTMSRR");
}

void loop() {
    gnss_readStream();
    delay(10);
}