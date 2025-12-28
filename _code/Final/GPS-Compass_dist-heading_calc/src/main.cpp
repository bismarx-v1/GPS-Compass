#include <Arduino.h>

const float lat_user = 50.1023364; // Users latitude
const float lon_user = 14.4444372; // Users longitude
const float lat_target = 50.12480503508704; // Targets latitude
const float lon_target = 14.50216610940226; // Targets longitude
float heading = 0.0; // Heading in degrees, info from BNO055

double degToRad(double degrees) {
    return degrees * (PI / 180.0);
}

double radToDeg(double radians) {
    return radians * (180.0 / PI);
}


float distance_calc() {
  // Haversine formula: 
  //a = sin²(Δφ/2) + cos φ1 ⋅ cos φ2 ⋅ sin²(Δλ/2)
  //c = 2 ⋅ atan2( √a, √(1−a) )
  //d = R ⋅ c
  double lat_user_rad = degToRad(lat_user);
  double lon_user_rad = degToRad(lon_user);
  double lat_target_rad = degToRad(lat_target);
  double lon_target_rad = degToRad(lon_target);

  const float R = 6371e3; // earths radius, metres
  const float phi1 = lat_user * PI/180; // phi, lambda in radians
  const float phi2 = lat_target * PI/180;
  const float delta_phi = (lat_target-lat_user) * PI/180;
  const float delta_lambda = (lon_target-lon_user) * PI/180;

  const float a = sin(delta_phi/2) * sin(delta_phi/2) + cos(phi1) * cos(phi2) * sin(delta_lambda/2) * sin(delta_lambda/2);
  // Represents how far apart the two points are on the unit sphere from value from: 0 to 1
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


void printMask(unsigned long long mask) {
    Serial.print("0x ");
    for (int i = 7; i >= 0; i--) {
        uint8_t b = (mask >> (i * 8)) & 0xFF;
        if (b < 0x10) Serial.print("0");
        Serial.print(b, HEX);
        Serial.print(" ");
    }
    Serial.println();
}


unsigned long long north_output(double headingDeg, double &northAngleOut, int &ledIndexOut) { 
    // Normalize heading
    while (headingDeg < 0) headingDeg += 360.0;
    while (headingDeg >= 360.0) headingDeg -= 360.0;

    // North relative angle
    double northAngle = 360.0 - headingDeg;
    if (northAngle >= 360.0) northAngle -= 360.0;

    const double DEG_PER_LED = 360.0 / 64.0;
    int ledIndex = (int)((northAngle + DEG_PER_LED / 2) / DEG_PER_LED);
    ledIndex %= 64;

    northAngleOut = northAngle;
    ledIndexOut = ledIndex;

    return (1ULL << ledIndex);
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

void setup() {
  Serial.begin(115200);

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
}


void loop() {

}
