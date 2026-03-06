#ifndef CHARGER_H
#define CHARGER_H

#include <Arduino.h>
#include <Wire.h>
#include <GPIO_MAP.h>
#include <math.h>

#define I2C_CHRGR_ADDR 0x6A

//Code edited with the help of AI 2026-03-05

// ==========================
// Register Definitions
// ==========================

#define REG02 0x02 //ADC control reg
#define REG00 0x00 //current control reg
#define REG0E 0x0E //battery monitoring reg
#define REG03 0x03 //OTG config reg 
#define REG12 0x12 //charge current reg
#define REG10 0x10 //temperature monitoring reg

#define CURRENT_MASK 0b00111111

// Predefined current settings (REG00 values)
#define CURR_0_5A  0b00001000
#define CURR_1A    0b00010010
#define CURR_1_5A  0b00011100
#define CURR_2A    0b00100010


// ==========================
// Low Level I2C Functions
// ==========================

inline uint8_t regSet(uint8_t reg, uint8_t val)
{
    Wire.beginTransmission((uint8_t)I2C_CHRGR_ADDR);
    Wire.write(reg);
    Wire.write(val);
    return Wire.endTransmission();
}

inline uint8_t regGet(uint8_t reg, uint8_t &val)
{
    Wire.beginTransmission((uint8_t)I2C_CHRGR_ADDR);
    Wire.write(reg);
    uint8_t err = Wire.endTransmission(false);
    if (err != 0) return err;

    uint8_t bytes = Wire.requestFrom((uint8_t)I2C_CHRGR_ADDR, (uint8_t)1);
    if (bytes != 1)
        return 4;   // custom error

    val = Wire.read();

    return 0;
}

float charger_readBatteryVoltage()
{
    uint8_t regVal;
    uint8_t err;

    err = regGet(REG0E, regVal);
    if (err)
        return -1.0;   // error indicator

    uint8_t vbatBits = regVal & 0x7F;   // bits 6:0

    float voltage = 2.304 + (vbatBits * 0.020);

    return voltage;
}

float charger_readChargeCurrent()
{
    uint8_t regVal;
    uint8_t err;

    err = regGet(REG12, regVal);
    if (err)
        return -1.0;   // error indicator

    uint8_t ichgBits = regVal & 0x7F;   // bits 6:0

    float current = ichgBits * 50.0;    // 50mA per step

    return current;
}



float charger_readBatteryTemp()
{
    uint8_t regVal;
    uint8_t err;

    err = regGet(REG10, regVal);
    if (err)
        return -100.0; // error indicator

    uint8_t tsBits = regVal & 0x7F;             
    float tspct = 0.21f + (tsBits * 0.00465f);  // 21% + 0.465% per bit 
    float Rp = (tspct * 5230.0f) / (1.0f - tspct);
    float R_NTC = 1.0f / ((1.0f / Rp) - (1.0f / 30100.0f));

    //Beta = 3950
    float tempK = 1.0f / (1.0f / 298.15f + (1.0f / 3950.0f) * log(R_NTC / 10000.0f));
    float tempC = tempK - 273.15f;

    return tempC;
}
// ==========================
// Charging Current Control
// ==========================

void charger_setCurrentRaw(uint8_t option)
{
    uint8_t regVal;
    uint8_t err;

    err = regGet(REG00, regVal);
    if (err) while (1);

    regVal = (regVal & ~CURRENT_MASK) | (option & CURRENT_MASK);

    err = regSet(REG00, regVal);
    if (err) while (1);
}


// ==========================
// Setup Function
// ==========================

void charger_setup()
{

    // Default current
    charger_setCurrentRaw(CURR_0_5A);
    regSet(REG02, 0b11110001); // Enable ADC
    delay(100); 
    regSet(REG03, 0b00011110); // Disable OTG mode, set SYS_MIN_VOLT to 3.7V
}

inline void charger_monitor() //debug monitor
{
    float vbat = charger_readBatteryVoltage();
    float ichg = charger_readChargeCurrent();
    float temp = charger_readBatteryTemp();

    Serial.print("Battery: ");
    Serial.print(vbat, 3);
    Serial.print(" V  |  Charge: ");
    Serial.print(ichg, 0);
    Serial.print(" mA  |  Temp: ");
    Serial.print(temp, 1);
    Serial.println(" C");

    delay(100);
}

#endif