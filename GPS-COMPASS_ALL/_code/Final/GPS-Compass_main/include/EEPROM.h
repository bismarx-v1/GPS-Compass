#ifndef I2C_EEPROM_H
#define I2C_EEPROM_H

#include <Wire.h>
#include <Arduino.h>

const uint8_t EEPROM_ADDR = 0x50;

template <typename T>
void eepromWrite(uint16_t memAddr, const T& value) {
    const byte* p = (const byte*)(const void*)&value;
    Wire.beginTransmission(EEPROM_ADDR);
    Wire.write((byte)(memAddr >> 8));   // MSB
    Wire.write((byte)(memAddr & 0xFF)); // LSB
    for (unsigned int i = 0; i < sizeof(value); i++) {
        Wire.write(*p++);
    }
    Wire.endTransmission();
    delay(5); // 5ms delay required for EEPROM internal write cycle
}

template <typename T>
void eepromRead(uint16_t memAddr, T& value) {
    byte* p = (byte*)(void*)&value;
    Wire.beginTransmission(EEPROM_ADDR);
    Wire.write((byte)(memAddr >> 8));   // MSB
    Wire.write((byte)(memAddr & 0xFF)); // LSB
    Wire.endTransmission();
    
    Wire.requestFrom((int)EEPROM_ADDR, (int)sizeof(value));
    for (unsigned int i = 0; i < sizeof(value); i++) {
        if (Wire.available()) {
            *p++ = Wire.read();
        }
    }
}

#endif