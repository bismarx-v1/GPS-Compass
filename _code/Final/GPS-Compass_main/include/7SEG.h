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

#define DIGIT_COUNT 3

#ifndef DIGIT_COUNT
  #define DIGIT_COUNT 8
#endif


class Display {
private:
  enum DriverRegisters {
    digit0     = 0x1,
    digit1     = 0x2,
    digit2     = 0x3,
    digit3     = 0x4,
    digit4     = 0x5,
    digit5     = 0x6,
    digit6     = 0x7,
    digit7     = 0x8,
    decodeMode = 0x9,
    intensity  = 0xA,
    scanLimit  = 0xB,
    shutdown   = 0xC,
    testMode   = 0xF,
  };

  // .-A-.
    // |...|
    // F...B
    // .-G-.
    // E...C
    // |...|
    // .-D-. (p)
    //
    // pABCDEFG
  struct Segments {
    bool point:1;
    bool top:1;
    bool topRight:1;
    bool botRight:1;
    bool bottom:1;
    bool botLeft:1;
    bool topLeft:1;
    bool centre:1;
  };
  
  const static uint8_t CHARLIST_LEN = 18;
  const Segments CHARLIST[CHARLIST_LEN] = {
    {0b01111110}, // '0'
    {0b00110000}, // '1'
    {0b01101101}, // '2'
    {0b01111001}, // '3'
    {0b00110011}, // '4'
    {0b01011011}, // '5'
    {0b01011111}, // '6'
    {0b01110000}, // '7'
    {0b01111111}, // '8'
    {0b01111011}, // '9'
    {0b01110111}, // 'A'
    {0b00011111}, // 'b'
    {0b00001101}, // 'c'
    {0b00111101}, // 'd'
    {0b01001111}, // 'E'
    {0b01000111}, // 'f'
    {0b00010111}, // 'h'
    {0b00000001}, // '-' - keep last
  };

  enum CharlistOffsets {
    NUM_0 = 0,
    NUM_1 = 1,
    NUM_2 = 2,
    NUM_3 = 3,
    NUM_4 = 4,
    NUM_5 = 5,
    NUM_6 = 6,
    NUM_7 = 7,
    NUM_8 = 8,
    NUM_9 = 9,
    CHR_A = 10,
    CHR_b = 11,
    CHR_c = 12,
    CHR_d = 13,
    CHR_E = 14,
    CHR_f = 15,
    CHR_h = 16,
  };


  Segments segmentBuffer[DIGIT_COUNT] = {0};

  void spiSend(DriverRegisters, uint8_t);
  
public:
  typedef uint8_t OneDigit;
  typedef uint8_t DigitPos;

  void setup(const uint8_t, const uint8_t, const uint8_t);
  void setDigit(OneDigit, DigitPos);
  void printNum(uint8_t, DigitPos, bool);
  void setDecimalP(DigitPos, bool);
  void clear();
  void flush();
};

void Display::setup(const uint8_t mosi, const uint8_t clk, const uint8_t cs) {
  pinMode(CS_SPI, OUTPUT);
  pinMode(CLK_SPI, OUTPUT);
  pinMode(MOSI_SPI, OUTPUT);

  digitalWrite(MOSI_SPI, LOW);
  digitalWrite(CLK_SPI, LOW);
  digitalWrite(CS_SPI, HIGH);

  spiSend(shutdown, 1);
  spiSend(testMode, 0);
  spiSend(decodeMode, 0);
  spiSend(scanLimit, 7);
  spiSend(intensity, 0xF);

  clear();
  flush();
};

void Display::spiSend(DriverRegisters reg, uint8_t val) {
  digitalWrite(CS_SPI, LOW);
  shiftOut(MOSI_SPI, CLK_SPI, MSBFIRST, reg);
  shiftOut(MOSI_SPI, CLK_SPI, MSBFIRST, val);
  digitalWrite(CS_SPI, HIGH);
};

void Display::clear() {
  memset(segmentBuffer, 0, sizeof(segmentBuffer[0]) * DIGIT_COUNT);
};

void Display::setDigit(OneDigit digit, DigitPos pos) {
  if(pos >= DIGIT_COUNT) {
    log_e("This digit pos doesn't exist.");
    return;
  }
  if(digit >= CHARLIST_LEN) {
    digit = CHARLIST_LEN - 1;
  }

  segmentBuffer[pos] = CHARLIST[digit];
};

void Display::printNum(uint8_t num, DigitPos pos, bool digitDirection) {
  
  
  int8_t dir = digitDirection * 2 - 1;
  setDigit(num / 100,      pos + 2 * dir);
  setDigit(num % 100 / 10, pos + 1 * dir);
  setDigit(num % 10,       pos);
};

void Display::setDecimalP(DigitPos pos, bool val) {
  if(pos >= DIGIT_COUNT) {
    log_e("This digit pos doesn't exist.");
    return;
  }
  
  segmentBuffer[pos].point = val;
};

#endif // 7SEG_H
