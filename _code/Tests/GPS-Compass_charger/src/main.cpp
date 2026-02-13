#include <Arduino.h>
#include <Wire.h>

#define I2C_CHRGR_ADDR 0x6A

#define INT_CHRG    5
#define SDA_I2C     6
#define SCL_I2C     7
#define DSEL        15
#define STAT        16

// ==========================
// Register Definitions
// ==========================
#define REG00 0x00
#define CURRENT_MASK 0b00111111

// Predefined current settings (REG00 values)
#define CURR_0_5A  0b00001000
#define CURR_1A    0b00010010
#define CURR_1_5A  0b00011100
#define CURR_2A    0b00100010


// ==========================
// Low Level I2C Functions
// ==========================

uint8_t regSet(uint8_t reg, uint8_t val)
{
    Wire.beginTransmission(I2C_CHRGR_ADDR);
    Wire.write(reg);
    Wire.write(val);
    return Wire.endTransmission();
}

uint8_t regGet(uint8_t reg, uint8_t &val)
{
    Wire.beginTransmission(I2C_CHRGR_ADDR);
    Wire.write(reg);
    uint8_t err = Wire.endTransmission(false);
    if (err != 0) return err;

    Wire.requestFrom(I2C_CHRGR_ADDR, 1);
    val = Wire.read();
    return 0;
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


// Optional: easier API (amps selection)
void charger_setCurrentAmp(float current)
{
    if (current <= 0.5f)
        charger_setCurrentRaw(CURR_0_5A);
    else if (current <= 1.0f)
        charger_setCurrentRaw(CURR_1A);
    else if (current <= 1.5f)
        charger_setCurrentRaw(CURR_1_5A);
    else
        charger_setCurrentRaw(CURR_2A);
}


// ==========================
// Setup Function
// ==========================

void charger_setup()
{
    pinMode(DSEL, INPUT);
    pinMode(INT_CHRG, INPUT);
    pinMode(STAT, INPUT);

    if (!Wire.begin(SDA_I2C, SCL_I2C, 40000))
    {
        while (1);
    }

    // Default current
    charger_setCurrentRaw(CURR_1A);
}


// ==========================
// MAIN
// ==========================

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println("Starting BQ25895M Charger Setup...");

    charger_setup();

    Serial.println("Charger Setup Complete.");

    // Example: Change current from main
    charger_setCurrentAmp(CURR_0_5A);
}

void loop()
{
}
