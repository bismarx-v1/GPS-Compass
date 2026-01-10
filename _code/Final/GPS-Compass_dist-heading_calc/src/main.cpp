#include <Arduino.h>

#define CLK_SR 40 //SCK
#define SI_SR 39 //SER
#define LATCH_SR 21 //RCK
unsigned long long mask = 0ULL;

const float lat_user = 50.1023364; // Users latitude
const float lon_user = 14.4444372; // Users longitude
const float lat_target = 50.12480503508704; // Targets latitude
const float lon_target = 14.50216610940226; // Targets longitude
float heading = 90.0; // Heading in degrees, info from BNO055

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
    int ledIndex = (int)((angleDeg + DEG_PER_LED / 2) / DEG_PER_LED) % 64;

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

void setup() {
  Serial.begin(115200);

  // initialize shift register pins
  pinMode(CLK_SR, OUTPUT);
  pinMode(SI_SR, OUTPUT);
  pinMode(LATCH_SR, OUTPUT);

  // initial clear
  mask = 0ULL;
  shift_8byte(mask);
  triggerBuffers();
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

  Serial.println();
  Serial.print("Heading [deg]: ");
  Serial.println(heading, 2);

  Serial.print("North angle [deg]: ");
  Serial.println(northAngle, 2);

  Serial.print("North LED index: ");
  Serial.println(northLed);

  Serial.print("North LED mask: ");
  printMask(northMask);

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

  // Combine masks (display both north and target)
  mask = northMask | targetMask;

  // send mask to shift register
  shift_8byte(mask);
  triggerBuffers();

  // wait so serial output is not constant
  delay(10000);
}
