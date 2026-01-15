#ifndef SEVEN_SEG_H
#define SEVEN_SEG_H

#include <Arduino.h>
#include <Wire.h>
#include "GPIO_MAP.h"

// =====================
// MAX7219 Register Map
// =====================
struct max7219Registers {
  const uint8_t digit0     = 0x1;
  const uint8_t digit1     = 0x2;
  const uint8_t digit2     = 0x3;
  const uint8_t digit3     = 0x4;
  const uint8_t digit4     = 0x5;
  const uint8_t digit5     = 0x6;
  const uint8_t digit6     = 0x7;
  const uint8_t digit7     = 0x8;
  const uint8_t decodeMode = 0x9;
  const uint8_t intensity  = 0xA;
  const uint8_t scanLimit  = 0xB;
  const uint8_t shutdown   = 0xC;
  const uint8_t testMode   = 0xF;

  const uint8_t digitArray[8] = {
    digit0, digit1, digit2, digit3,
    digit4, digit5, digit6, digit7
  };


  const uint8_t digitIndexArray[8] = {0,1,2,3,4,5,6,7};

  /**
   *  -A-
   * |   |
   * F   B
   *  -G-
   * E   C
   * |   |
   *  -D-  (p)
   *
   * pABCDEFG
   */
  const uint8_t numberCharArray[10] = {
    0b01111110, // 0
    0b00110000, // 1
    0b01101101, // 2
    0b01111001, // 3
    0b00110011, // 4
    0b01011011, // 5
    0b01011111, // 6
    0b01110000, // 7
    0b01111111, // 8
    0b01111011  // 9
  };

  const uint8_t negativeSign = 0b00000001;
};

// =====================
// Module-local storage
// =====================
static uint8_t segmentsArrayObjGlobal[8] = {0};
static max7219Registers maxRegisters;

// =====================
// SPI helpers
// =====================
static inline void spiSend(uint8_t reg, uint8_t val) {
  digitalWrite(CS_SPI, LOW);
  shiftOut(MOSI_SPI, CLK_SPI, MSBFIRST, reg);
  shiftOut(MOSI_SPI, CLK_SPI, MSBFIRST, val);
  digitalWrite(CS_SPI, HIGH);
}

static inline void clearSpi() {
  for (uint8_t i = 0; i < 8; i++) {
    spiSend(maxRegisters.digitArray[i], 0);
  }
}

// =====================
// Display control
// =====================
static inline void display_setup() {
  pinMode(CS_SPI, OUTPUT);
  pinMode(CLK_SPI, OUTPUT);
  pinMode(MOSI_SPI, OUTPUT);

  digitalWrite(MOSI_SPI, LOW);
  digitalWrite(CLK_SPI, LOW);
  digitalWrite(CS_SPI, HIGH);

  spiSend(maxRegisters.shutdown, 1);
  spiSend(maxRegisters.testMode, 0);
  spiSend(maxRegisters.decodeMode, 0);
  spiSend(maxRegisters.scanLimit, 7);
  spiSend(maxRegisters.intensity, 0xF);

  clearSpi();
}

static inline void push() {
  for (uint8_t i = 0; i < 8; i++) {
    spiSend(
      maxRegisters.digitArray[maxRegisters.digitIndexArray[i]],
      segmentsArrayObjGlobal[i]
    );
  }
}

static inline void displayDigit(uint8_t pos, uint8_t num, uint8_t decimalPoint) {
  if (pos > 7) return;

  decimalPoint = decimalPoint ? 0x80 : 0x00;
  segmentsArrayObjGlobal[pos] =
    maxRegisters.numberCharArray[num % 10] | decimalPoint;
}

#endif // 7SEG_H
