#ifndef DIST_CALC_H
#define DIST_CALC_H

#include <Arduino.h>
#include "GNSS_NMEA.h"
#include "WEBSERVER.h"
#include "SYSTEM_STATE.h"

//Code edited with the help of AI 2026-03-05

// float current_lat = 50.102361; //test coords
// float current_lon = 14.444472;

/* ================= HELPERS ================= */

inline double degToRad(double degrees) {
    return degrees * (PI / 180.0);
}

inline double radToDeg(double radians) {
    return radians * (180.0 / PI);
}

/* ================= DISTANCE TO TARGET ================= */

inline float distance_to_target() {
    // 1. Fetch current coordinates dynamically every time function is called
    float current_lat = gnss_getLat();
    float current_lon = gnss_getLon();

    const float R = 6371e3; // Earth radius in meters

    const float phi1 = current_lat * PI/180;
    const float phi2 = target_position.lat * PI/180;
    const float delta_phi = (target_position.lat - current_lat) * PI/180;
    const float delta_lambda = (target_position.lon - current_lon) * PI/180;

    const float a =
        sin(delta_phi/2) * sin(delta_phi/2) +
        cos(phi1) * cos(phi2) *
        sin(delta_lambda/2) * sin(delta_lambda/2);

    const float c = 2 * atan2(sqrt(a), sqrt(1-a));

    return R * c;
}

/* ================= BEARING ================= */

inline double bearing_to_target() {
    
    float current_lat = gnss_getLat();
    float current_lon = gnss_getLon();

    double phi1 = degToRad(current_lat);
    double phi2 = degToRad(target_position.lat);
    double delta_lambda = degToRad(target_position.lon - current_lon);

    double y = sin(delta_lambda) * cos(phi2);
    double x = cos(phi1)*sin(phi2) -
               sin(phi1)*cos(phi2)*cos(delta_lambda);

    double theta = atan2(y, x);
    double bearing = radToDeg(theta);

    if (bearing < 0) {
        bearing += 360.0;
    }

    return bearing;
}

#endif