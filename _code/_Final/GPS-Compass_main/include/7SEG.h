#ifndef SEVENSEG_H
#define SEVENSEG_H

#include <Arduino.h>
#include <GPIO_MAP.h>

//Code edited with the help of AI 2026-03-05

// -----------------------------------------------------------------------------
// MAX7219 / 8x7-segment display helper definitions
// -----------------------------------------------------------------------------
// This module uses the MAX7219 driver over SPI to control a chain of 7-segment
// digits. The MAX7219 expects a 16-bit command (8-bit register + 8-bit value).
// The `segmentsArrayObjGlobal` buffer holds the segment bits for each digit.

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

  const uint8_t digitArray[8] = {digit0, digit1, digit2, digit3, digit4, digit5, digit6, digit7};

  // This array maps the logical digit index (0-7) in segmentsArrayObjGlobal to the
  // actual MAX7219 digit registers. Change it if the physical digit ordering is different.
  const uint8_t digitIndexArray[8] = {0, 1, 2, 3, 4, 5, 6, 7};

  /**
	 * Seven-segment layout (pABCDEFG):
	 *	 -A-
	 *	|   |
	 *	F   B
	 *	 -G-
	 *	E   C
	 *	|   |
	 *	 -D-  (p)
	 *
	 * The bit order in the byte is: p G F E D C B A
	 */
  // pABCDEFG
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
  }; // 0-9

  // Bitmask for the negative sign (segment A only)
  const uint8_t negativeSign = 0b00000001;
};

// Global buffer for the 8 digits on the display.
// Each element is a byte where individual bits control the segments.
uint8_t segmentsArrayObjGlobal[8];

// Encapsulates the MAX7219 register addresses and segment/character mapping.
max7219Registers maxRegisters;

void spiSend(uint8_t reg, uint8_t val) {
  digitalWrite(CS_SPI, 0);
  shiftOut(MOSI_SPI, CLK_SPI, MSBFIRST, reg);
  shiftOut(MOSI_SPI, CLK_SPI, MSBFIRST, val);
  digitalWrite(CS_SPI, 1);
}

void clearSpi() {
  spiSend(maxRegisters.digit7, 0);
  spiSend(maxRegisters.digit6, 0);
  spiSend(maxRegisters.digit5, 0);
  spiSend(maxRegisters.digit4, 0);
  spiSend(maxRegisters.digit3, 0);
  spiSend(maxRegisters.digit2, 0);
  spiSend(maxRegisters.digit1, 0);
  spiSend(maxRegisters.digit0, 0);
}

void display_setup() {
  pinMode(CS_SPI, OUTPUT);
  pinMode(CLK_SPI, OUTPUT);
  pinMode(MOSI_SPI, OUTPUT);
  digitalWrite(MOSI_SPI, 0);
  digitalWrite(CLK_SPI, 0);
  digitalWrite(CS_SPI, 1);

  spiSend(maxRegisters.shutdown, 1);     // Exit shutdown.
  spiSend(maxRegisters.testMode, 0);     // Tet mode off.
  spiSend(maxRegisters.decodeMode, 0);   // Turn off decode.
  spiSend(maxRegisters.scanLimit, 7);    // Activeate all digits.
  spiSend(maxRegisters.intensity, 0xF);  // Intensity to max.
  clearSpi();
}

void push() {
  for(uint8_t i = 0; i < 8; i++) {
    spiSend(maxRegisters.digitArray[maxRegisters.digitIndexArray[i]], segmentsArrayObjGlobal[i]);
  }
}

void displayDigit(uint8_t pos, uint8_t num, uint8_t decimalPoint) {
  if(pos > 7 || pos < 0) {
    return;
  }

  if(decimalPoint != 1) {
    decimalPoint = 0;
  }
  segmentsArrayObjGlobal[pos] = maxRegisters.numberCharArray[num % 10] | 0b10000000 * decimalPoint;
}

 void convertDistanceToBuffer(uint32_t distance)
{
    // Clear used digits (0–2)
    segmentsArrayObjGlobal[0] = 0;
    segmentsArrayObjGlobal[1] = 0;
    segmentsArrayObjGlobal[2] = 0;

    // 1000m – 9999m  →  X.X km
    if (distance > 999 && distance < 10000)
    {
        uint32_t temp = distance / 10;

        displayDigit(2, temp % 10, 0);
        displayDigit(1, (temp / 10) % 10, 0);
        displayDigit(0, temp / 100, 1);
    }

    // 10000m – 99999m  →  XX.X km
    else if (distance > 9999 && distance < 100000)
    {
        uint32_t temp = distance / 100;

        displayDigit(2, temp % 10, 0);
        displayDigit(1, (temp / 10) % 10, 1);
        displayDigit(0, temp / 100, 0);
    }

    // >= 100000m  →  XXX km
    else if (distance >= 100000)
    {
        uint32_t temp = distance / 1000;

        displayDigit(2, temp % 10, 1);
        displayDigit(1, (temp / 10) % 10, 0);
        displayDigit(0, temp / 100, 0);
    }

    // < 1000m → meters
    else
    {
        displayDigit(2, distance % 10, 0);
        displayDigit(1, (distance / 10) % 10, 0);
        displayDigit(0, distance / 100, 0);
    }
}

#endif // SEVENSEG_H