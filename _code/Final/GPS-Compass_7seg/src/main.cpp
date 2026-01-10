#include <Arduino.h>

#define GPIO_SPI_CS_DRIVER 10
#define GPIO_SPI_CLK 12
#define GPIO_SPI_MOSI 11
float distance_float = 123.0;
int distance = round(distance_float);

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

  // This array maps the registers to the digits in the segment array. Index is array index, number is digit register.
  const uint8_t digitIndexArray[8] = {0,1,2,3,4,5,6,7}; // edit with new display (3digit)

  /**
	 *  -A-
	 * |   |
	 * F   B
	 *  -G-
	 * E   C
	 * |   |
	 *  -D-  (p)
	 */
  // pABCDEFG
  const uint8_t numberCharArray[10] = {0b01111110, 0b00110000, 0b01101101, 0b01111001, 0b00110011,
                                       0b01011011, 0b01011111, 0b01110000, 0b01111111, 0b01111011}; // 0-9
  const uint8_t negativeSign        = 0b00000001; 
};

uint8_t          segmentsArrayObjGlobal[8];
max7219Registers maxRegisters;

void spiSend(uint8_t reg, uint8_t val) {
  digitalWrite(GPIO_SPI_CS_DRIVER, 0);
  shiftOut(GPIO_SPI_MOSI, GPIO_SPI_CLK, MSBFIRST, reg);
  shiftOut(GPIO_SPI_MOSI, GPIO_SPI_CLK, MSBFIRST, val);
  digitalWrite(GPIO_SPI_CS_DRIVER, 1);
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
  pinMode(GPIO_SPI_CS_DRIVER, OUTPUT);
  pinMode(GPIO_SPI_CLK, OUTPUT);
  pinMode(GPIO_SPI_MOSI, OUTPUT);
  digitalWrite(GPIO_SPI_MOSI, 0);
  digitalWrite(GPIO_SPI_CLK, 0);
  digitalWrite(GPIO_SPI_CS_DRIVER, 1);

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
  //segmentsArrayObjGlobal[maxRegisters.digitIndexArray[pos]] = maxRegisters.numberCharArray[num % 10] | 0b10000000 * decimalPoint;
  segmentsArrayObjGlobal[pos] = maxRegisters.numberCharArray[num % 10] | 0b10000000 * decimalPoint;
}

void setup() {
  display_setup();
}
//ternary operator in C++s
void loop() {
  if(distance > 999 && distance < 10000) { //eg: 1000m = 1.0km
    int distance_temp = distance / 10; // Convert to kilometers.
    displayDigit(2, distance_temp % 10, 0);
    displayDigit(1, distance_temp % 100 / 10, 0);
    displayDigit(0, distance_temp / 100, 1);
    push();
  }

  //1000m   = 1.,0,0
  //10000m  = 1,0.,0
  //100000m = 1,0,0.

  else if(distance >  9999 && distance < 100000) { //eg: 10000m = 10.0km
    int distance_temp = distance / 100; // Convert to kilometers.
    displayDigit(2, distance_temp % 10, 0);
    displayDigit(1, distance_temp % 100 / 10, 1);
    displayDigit(0, distance_temp / 100, 0);
    push();
  }
  else if(distance > 99999) {
    int distance_temp = distance / 1000; // Convert to kilometers.
    displayDigit(2, distance_temp % 10, 1);
    displayDigit(1, distance_temp % 100 / 10, 0);
    displayDigit(0, distance_temp / 100, 0);
    push();
  }
  else {
    displayDigit(2, distance % 10, 0); //meters
    displayDigit(1, distance % 100 / 10, 0);
    displayDigit(0, distance / 100, 0);
    push();
    }
}