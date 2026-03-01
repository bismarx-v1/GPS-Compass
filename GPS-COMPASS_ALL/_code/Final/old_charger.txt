#ifndef CHARGER_H
#define CHARGER_H

#include <Arduino.h>
#include <Wire.h>
#include "GPIO_MAP.h"

#define I2C_CHRGR_ADDR 0x6a

class Bq25895m {
private:
    enum RegAddr {
        REG00   = 0,
    };

    enum CurrentOption {
        MASK        = 0b00111111,
        CURR_1A     = 0b00010010,   // 1A
        CURR_1A5    = 0b00011100,   // 1.5A
        CURR_2A     = 0b00100010,   // 2A
    };

    uint8_t regSet(RegAddr, uint8_t);
    uint8_t regGet(RegAddr, uint8_t&);
    void setCurrent(CurrentOption);
    
public:
    void setup();
};


void Bq25895m::setup() {
    pinMode(DSEL, INPUT);
    pinMode(INT_CHRG, INPUT);
    pinMode(STAT, INPUT);
    if(!Wire.begin(SDA_I2C, SCL_I2C, 40000)) {while (1) {log_e("Wire err: begin.");}}

    setCurrent(CURR_1A5);

}

uint8_t Bq25895m::regSet(RegAddr reg, uint8_t val) {
    Wire.beginTransmission(I2C_CHRGR_ADDR);
    Wire.write(reg);
    Wire.write(val);
    return Wire.endTransmission();
}

uint8_t Bq25895m::regGet(RegAddr reg, uint8_t &val) {
    Wire.beginTransmission(I2C_CHRGR_ADDR);
    Wire.write(reg);
    uint8_t tmp = Wire.endTransmission(false);
    if(tmp != 0) {return tmp;}

    Wire.requestFrom(I2C_CHRGR_ADDR, sizeof(uint8_t));
    val = Wire.read();
    return Wire.endTransmission();
}

void Bq25895m::setCurrent(CurrentOption option) {
    uint8_t regVal, regErr;
    if(regErr = regGet(REG00, regVal)) {while (1) {log_e("Wire err: regGet: %u.", regErr);}}
    regVal = (regVal & ~CurrentOption::MASK) | (option & CurrentOption::MASK);
    if(regErr = regSet(REG00, regVal)) {while (1) {log_e("Wire err: regSet: %u.", regErr);}}
}

#endif // CHARGER_H
