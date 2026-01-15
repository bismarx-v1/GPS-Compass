#ifndef DIST_CALC_H
#define DIST_CALC_H

#include <Arduino.h>
#include <Wire.h>
#include "GPIO_MAP.h"

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

#endif // DIST_CALC_H