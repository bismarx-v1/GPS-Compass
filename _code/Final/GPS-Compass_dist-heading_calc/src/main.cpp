#include <Arduino.h>
//#include <GPIO_MAP.h>

#define GPIO_SPI_CS_DRIVER 10
#define GPIO_SPI_CLK 12
#define GPIO_SPI_MOSI 11

#define CLK_SR 40 //SCK
#define SI_SR 39 //SER
#define LATCH_SR 21 //RCK
unsigned long long mask = 0ULL;

const float lat_user = 50.1023364; // Users latitude
const float lon_user = 14.4444372; // Users longitude
const float lat_target = 50.12480503508704; // Targets latitude
const float lon_target = 14.50216610940226; // Targets longitude
float heading = 0.0; // Heading in degrees, info from BNO055

// North LED blinking state
const unsigned long NORTH_BLINK_INTERVAL = 1000; // milliseconds
unsigned long lastNorthToggle = 0;
bool northLedState = true; // start ON


//distance and bearing part


double degToRad(double degrees) {
    return degrees * (PI / 180.0);
}

double radToDeg(double radians) {
    return radians * (180.0 / PI);
}

float distance_calc() {
  // Haversine formula
  const float R = 6371e3; // earths radius, metres
  const float phi1 = lat_user * PI/180; // phi, lambda in radians
  const float phi2 = lat_target * PI/180;
  const float delta_phi = (lat_target-lat_user) * PI/180;
  const float delta_lambda = (lon_target-lon_user) * PI/180;

  const float a = sin(delta_phi/2) * sin(delta_phi/2) + cos(phi1) * cos(phi2) * sin(delta_lambda/2) * sin(delta_lambda/2);
  const float c = 2 * atan2(sqrt(a), sqrt(1-a)); // angular distance in radians
  const float distance = R * c; // in metres 
  return distance;
}

unsigned long long angleToLedMask(double angleDeg, int &ledIndexOut) {
    while (angleDeg < 0) angleDeg += 360.0;
    while (angleDeg >= 360.0) angleDeg -= 360.0;

    const double DEG_PER_LED = 360.0 / 64.0;
    int ledIndex_logical = (int)((angleDeg + DEG_PER_LED / 2) / DEG_PER_LED) % 64;
    int ledIndex = (ledIndex_logical + 23) % 64; // Shift to match physical wiring
    ledIndexOut = ledIndex;
    return (1ULL << ledIndex);
}

void printMask(unsigned long long led_mask) {
    Serial.print("0x ");
    for (int i = 7; i >= 0; i--) {
        uint8_t b = (led_mask >> (i * 8)) & 0xFF;
        if (b < 0x10) Serial.print("0");
        Serial.print(b, HEX);
        Serial.print(" ");
    }
    Serial.println();
}

double bearingToTarget(float lat1, float lon1, float lat2, float lon2) { //angle from north to target
    double phi1 = degToRad(lat1);
    double phi2 = degToRad(lat2);
    double delta_lambda = degToRad(lon2 - lon1);

    double y = sin(delta_lambda) * cos(phi2);
    double x = cos(phi1)*sin(phi2) - sin(phi1)*cos(phi2)*cos(delta_lambda);
    double theta = atan2(y, x);

    double bearing = radToDeg(theta);
    if (bearing < 0) bearing += 360.0;
    return bearing;
}



//shift register part



// Shift register helpers
void shift_bit(bool bit) {
  digitalWrite(SI_SR, bit ? HIGH : LOW);
  delayMicroseconds(1); 
  digitalWrite(CLK_SR, HIGH);
  delayMicroseconds(1); 
  digitalWrite(CLK_SR, LOW);
  delayMicroseconds(1);
}

void shift_byte(uint8_t chomp) {
  for(uint8_t bit_index = 0; bit_index < 8; bit_index++) {
    shift_bit((chomp >> bit_index) & 1);
  }
}

void shift_8byte(unsigned long long nom) {
  for(uint8_t byte_index = 0; byte_index < 8; byte_index++){
    uint8_t b = (nom >> (byte_index * 8)) & 0xFF;
    shift_byte(b);
  }
}

void triggerBuffers() {
  digitalWrite(LATCH_SR, HIGH);
  delayMicroseconds(1); 
  digitalWrite(LATCH_SR, LOW);
  delayMicroseconds(1);
}


//7seg part

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

#include "bq25895m.h" // HERE
Bq25895m bq25895mObj; // HERE

void setup() {
  Serial.begin(115200);

  bq25895mObj.setup(); // HERE
  

  // initialize shift register pins
  pinMode(CLK_SR, OUTPUT);
  pinMode(SI_SR, OUTPUT);
  pinMode(LATCH_SR, OUTPUT);

  // initial clear
  mask = 0ULL;
  shift_8byte(mask);
  triggerBuffers();

  // 7segment display setup
  display_setup();
}

void loop() {
  // Recompute masks and print values periodically
  // Distance
  Serial.println();
  Serial.print("Distance [m]: ");
  Serial.println(distance_calc(), 2);

  // -------- NORTH --------
  double northAngle = 360.0 - heading;
  if (northAngle >= 360.0) northAngle -= 360.0;

  int northLed;
  unsigned long long northMask = angleToLedMask(northAngle, northLed);

  // Toggle north LED state every NORTH_BLINK_INTERVAL
  unsigned long now = millis();
  if (now - lastNorthToggle >= NORTH_BLINK_INTERVAL) {
    lastNorthToggle = now;
    northLedState = !northLedState;
  }
  unsigned long long displayedNorthMask = northLedState ? northMask : 0ULL;

  Serial.println();
  Serial.print("Heading [deg]: ");
  Serial.println(heading, 2);

  Serial.print("North angle [deg]: ");
  Serial.println(northAngle, 2);

  Serial.print("North LED index: ");
  Serial.println(northLed);

  Serial.print("North LED mask: ");
  printMask(displayedNorthMask);

  // -------- TARGET --------
  double bearing = bearingToTarget(lat_user, lon_user, lat_target, lon_target);
  double relativeAngle = bearing - heading;
  if (relativeAngle < 0) relativeAngle += 360.0;

  int targetLed;
  unsigned long long targetMask = angleToLedMask(relativeAngle, targetLed);

  Serial.println();
  Serial.print("Bearing to target [deg]: ");
  Serial.println(bearing, 2);

  Serial.print("Relative angle to target [deg]: ");
  Serial.println(relativeAngle, 2);

  Serial.print("Target LED index: ");
  Serial.println(targetLed);

  Serial.print("Target LED mask: ");
  printMask(targetMask);

  // Combine masks (display both north and target). Use displayedNorthMask to blink north.
  mask = displayedNorthMask | targetMask;

  // send mask to shift register
  shift_8byte(mask);
  triggerBuffers();

  int distance = round(distance_calc());

  if(distance > 999 && distance < 10000) { //eg: 1000m = 1.0km
    int distance_kilo = distance / 10; // Convert to kilometers.
    displayDigit(2, distance_kilo % 10, 0);
    displayDigit(1, distance_kilo % 100 / 10, 0);
    displayDigit(0, distance_kilo / 100, 1);
    push();
  }

  //1000m   = 1.,0,0
  //10000m  = 1,0.,0
  //100000m = 1,0,0.
  
  else if(distance >  9999 && distance < 100000) { //eg: 10000m = 10.0km
    int distance_kilo = distance / 100; // Convert to kilometers.
    displayDigit(2, distance_kilo % 10, 0);
    displayDigit(1, distance_kilo % 100 / 10, 1);
    displayDigit(0, distance_kilo / 100, 0);
    push();
  }
  else if(distance > 99999) {
    int distance_kilo = distance / 1000; // Convert to kilometers.
    displayDigit(2, distance_kilo % 10, 1);
    displayDigit(1, distance_kilo % 100 / 10, 0);
    displayDigit(0, distance_kilo / 100, 0);
    push();
  }
  else {
    displayDigit(2, distance % 10, 0); //meters
    displayDigit(1, distance % 100 / 10, 0);
    displayDigit(0, distance / 100, 0);
    push();
    }

  // wait so serial output is not constant
  delay(100);
  heading += 2;
}