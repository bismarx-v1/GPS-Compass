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

// float charger_readBatteryTemp()
// {
//     uint8_t regVal;
//     uint8_t err;

//     err = regGet(REG10, regVal);
//     if (err)
//         return -100.0; // error indicator

//     uint8_t tsBits = regVal & 0x7F;             // bits 6:0
//     float tspct = 0.21 + tsBits * 0.00465;     // 21% + 0.465% per bit

//     float V_TS = tspct * 4.8;                // TS pin voltage

//     // Compute NTC resistance
//     float R_NTC = (V_TS * (5230 + 30100) - 4.8 * 30100) / (4.8 - V_TS);

//     // Beta equation
//     float tempK = 1.0 / (1.0 / 298.15 + (1.0 / 3950) * log(R_NTC / 10000));
//     float tempC = tempK - 273.15;

//     return tempC;
// }

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
    if (current <= 0.5)
        charger_setCurrentRaw(CURR_0_5A);
    else if (current <= 1.0)
        charger_setCurrentRaw(CURR_1A);
    else if (current <= 1.5)
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
    charger_setCurrentRaw(CURR_0_5A);
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
    charger_setCurrentAmp(1.0);
    regSet(REG02, 0b11110001); // Enable ADC, set to battery voltage mode
    regSet(REG03, 0b00011110); // Disable OTG mode, set SYS_MIN_VOLT to 3.7V

}

void loop()
{
    float vbat = charger_readBatteryVoltage();
    float ichg = charger_readChargeCurrent();
    //float temp = charger_readBatteryTemp();

    if (vbat > 0)
    {
        Serial.print("Battery Voltage: ");
        Serial.print(vbat, 3);
        Serial.println(" V");
    }
    else
    {
        Serial.println("Voltage read error.");
    }

    if (ichg >= 0)
    {
        Serial.print("Charge Current: ");
        Serial.print(ichg);
        Serial.println(" mA");
    }
    else
    {
        Serial.println("ICHG read error");
    }

    delay(1000);
}