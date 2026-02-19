#include <Arduino.h>
#include <Wire.h>

#define I2C_GNSS_ADDR   0x3A  
#define SDA_PIN         6     
#define SCL_PIN         7     
#define REG_NMEA_STREAM 0xFF  
#define TESEO_DUMMY     0xFF  

// ==========================
// CORE FUNCTIONS
// ==========================

uint8_t calculateChecksum(const char* str) {
    uint8_t checksum = 0;
    while (*str) checksum ^= *str++;
    return checksum;
}

void gnss_sendCommand(const char* rawCommand) {
    char fullBuffer[128];
    uint8_t checksum = calculateChecksum(rawCommand);
    sprintf(fullBuffer, "$%s*%02X\r\n", rawCommand, checksum);

    Serial.print("Configuring: "); 
    Serial.print(fullBuffer);

    Wire.beginTransmission(I2C_GNSS_ADDR);
    Wire.write(REG_NMEA_STREAM); 
    Wire.write((uint8_t*)fullBuffer, strlen(fullBuffer));
    uint8_t error = Wire.endTransmission();
    
    if (error != 0) {
        Serial.print("I2C Error: "); Serial.println(error);
    }
    delay(100); 
}

void gnss_readStream() {
    // Request 32 bytes from the GNSS module
    // We check the return value to ensure the read was successful
    uint8_t bytesRequested = 32;
    uint8_t bytesReceived = Wire.requestFrom((uint8_t)I2C_GNSS_ADDR, bytesRequested);
    
    // Only proceed if the module actually responded (bytesReceived > 0)
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
// SETUP & LOOP
// ==========================

void setup() {
    Serial.begin(115200);
    while (!Serial);
    
    Serial.println("\n--- Teseo-LIV3R ESP32-S3 (Custom PCB) ---");

    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(400000); 
    
    // FIX: Increase the I2C timeout to 100ms to prevent "Error -1" 
    // during internal GNSS processing.
    Wire.setTimeOut(100); 

    // 1. Set Constellations (GPS+GLO+GAL+BDS = 15)
    gnss_sendCommand("PSTMSETCONSTMASK,15");

    // 2. Set Fix Rate (1Hz)
    gnss_sendCommand("PSTMSETPAR,1303,1.0");

    // 3. Reset to apply changes
    gnss_sendCommand("PSTMSRR");

    // FIX: Wait for the module to reboot before starting the loop
    Serial.println("Waiting for module reboot...");
    delay(2000); 

    Serial.println("--- Stream Ready ---\n");
}

void loop() {
    gnss_readStream();
    delay(10); 
}