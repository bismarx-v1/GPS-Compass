#ifndef SEVEN_SEG_H
#define SEVEN_SEG_H

#include <Arduino.h>
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
 uint8_t segmentsArrayObjGlobal[8] = {0};
 max7219Registers maxRegisters;

// =====================
// SPI helpers
// =====================

/**
 * @brief [INTERNAL] Send a register write command to MAX7219 via SPI
 * 
 * Internal helper function used only within this module.
 * Not intended for external use.
 * 
 * Communicates with the MAX7219 by:
 * 1. Pulling CS low (chip select)
 * 2. Shifting out the register address
 * 3. Shifting out the register value
 * 4. Pulling CS high to latch the data
 * 
 * @param reg The MAX7219 register address to write to
 * @param val The 8-bit value to write to the register
 */
inline void spiSend(uint8_t reg, uint8_t val) {
  digitalWrite(CS_SPI, LOW);
  shiftOut(MOSI_SPI, CLK_SPI, MSBFIRST, reg);
  shiftOut(MOSI_SPI, CLK_SPI, MSBFIRST, val);
  digitalWrite(CS_SPI, HIGH);
}

/**
 * @brief [INTERNAL] Clear all 8 digit displays
 * 
 * Internal helper function used only within this module.
 * Not intended for external use.
 * 
 * Writes 0x00 to each digit register, turning off all LED segments on all digits.
 */
inline void clearSpi() {
  for (uint8_t i = 0; i < 8; i++) {
    spiSend(maxRegisters.digitArray[i], 0);
  }
}

// =====================
// Display control
// =====================

/**
 * @brief [EXTERNAL API] Initialize the MAX7219 display controller and configure GPIO
 * 
 * Public function intended for external use during system initialization.
 * Must be called once before using any other display functions.
 * 
 * This function:
 * 1. Configures SPI GPIO pins (CS, CLK, MOSI) as outputs
 * 2. Sets initial pin states
 * 3. Initializes MAX7219 registers for normal operation
 * 4. Sets display intensity to maximum (0xF)
 * 5. Clears all digit displays
 * 
 * @note Should be called once during system initialization before using the display.
 */
inline void display_setup() {
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

/**
 * @brief [EXTERNAL API] Push the current segment buffer to all 8 digits on the display
 * 
 * Public function intended for external use to update the physical display.
 * 
 * Sends the contents of segmentsArrayObjGlobal to the MAX7219,
 * updating the physical LED display with the buffered values.
 * Call this after modifying one or more digits with displayDigit() to see changes.
 * 
 * @note Recommended usage pattern:
 *       - Call displayDigit() one or more times to update the buffer
 *       - Call push() once to send all changes to the display
 */
inline void push() {
  for (uint8_t i = 0; i < 8; i++) {
    spiSend(
      maxRegisters.digitArray[maxRegisters.digitIndexArray[i]],
      segmentsArrayObjGlobal[i]
    );
  }
}

/**
 * @brief [EXTERNAL API] Set a digit in the display buffer with optional decimal point
 * 
 * Public function intended for external use to update digit values.
 * 
 * Updates the segment buffer for a specific digit position. The display
 * is not updated until push() is called.
 * 
 * @param pos Position of the digit (0-7, left to right)
 * @param num Number to display (0-9; values > 9 will use modulo 10)
 * @param decimalPoint Boolean flag: 1 to enable decimal point, 0 to disable
 * 
 * @note Invalid positions (> 7) are silently ignored
 * @note Typical usage:
 *       displayDigit(0, 1, 0);  // Set digit 0 to '1' without decimal point
 *       displayDigit(1, 2, 1);  // Set digit 1 to '2' with decimal point
 *       push();                 // Send updates to physical display
 */
inline void displayDigit(uint8_t pos, uint8_t num, uint8_t decimalPoint) {
  if (pos > 7) return;

  decimalPoint = decimalPoint ? 0x80 : 0x00;
  segmentsArrayObjGlobal[pos] =
    maxRegisters.numberCharArray[num % 10] | decimalPoint;
}

#endif // 7SEG_H