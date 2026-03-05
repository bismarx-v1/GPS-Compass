#ifndef IMU_H
#define IMU_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <Wire.h>
#include "GPIO_MAP.h"
#include "EEPROM.h"

// ==========================
// Configuration
// ==========================

#define BNO055_I2C_ADDR 0x29
#define EEPROM_CALIB_ADDR 0x0010
#define CALIB_MAGIC 0xACE1 // ID to verify valid data

//#define BNO055_ID       55 // Default ID is 0xA0

static Adafruit_BNO055 bno = Adafruit_BNO055(BNO055_ID, BNO055_I2C_ADDR, &Wire);

struct BNO_Calib_Profile {
    uint16_t magic;
    adafruit_bno055_offsets_t offsets;
};

// ==========================
// Calibration Storage
// ==========================

inline void imu_saveCalibration() {
    BNO_Calib_Profile profile;
    profile.magic = CALIB_MAGIC;
    
    // Get offsets from sensor hardware
    if (bno.getSensorOffsets(profile.offsets)) {
        eepromWrite(EEPROM_CALIB_ADDR, profile);
        Serial.println("IMU: Calibration profile saved to EEPROM.");
    }
}

inline bool imu_loadCalibration() {
    BNO_Calib_Profile profile;
    eepromRead(EEPROM_CALIB_ADDR, profile);

    // Check if the EEPROM actually contains valid data
    if (profile.magic != CALIB_MAGIC) {
        Serial.println("IMU: No valid calibration found in EEPROM.");
        return false;
    }

    // Apply the offsets to the BNO055
    bno.setSensorOffsets(profile.offsets);
    Serial.println("IMU: Calibration profile restored.");
    return true;
}

// ==========================
// Core IMU Functions
// ==========================

// Returns the Heading in degrees (0.0 to <360.0) relative to the PCB's orientation
inline float imu_getHeading()
{
    imu::Quaternion q = bno.getQuat();

    float w = q.w();
    float x = q.x();
    float y = q.y();
    float z = q.z();

    // 1. Rotate the World North vector (1,0,0) into the sensor's local frame
    // This uses the conjugate of the quaternion (w, -x, -y, -z) to reverse the rotation
    float local_x = (w * w) + (x * x) - (y * y) - (z * z);
    float local_y = 2.0 * ((x * y) - (w * z));

    // 2. Calculate the angle on the XY plane of the PCB
    float heading_rad = atan2(local_y, local_x);
    float heading_deg = heading_rad * (180.0 / PI);

    // 3. Normalize to 0 - 360 degrees
    if (heading_deg < 0) {
        heading_deg += 360.0;
    }

    // Optional: If your LEDs spin the wrong way, invert the angle:
    // return 360.0 - heading_deg;
    
    return heading_deg;
}

inline void imu_getCalibration(uint8_t &sys, uint8_t &gyro, uint8_t &accel, uint8_t &mag)
{
    bno.getCalibration(&sys, &gyro, &accel, &mag);
}

// ==========================
// Setup & Monitor
// ==========================`

inline void imu_setup()
{
    if (!bno.begin()) {
        Serial.println("BNO055 Init Failed");
        while (1);
    }

    imu_loadCalibration();

    delay(100);
}

inline void imu_monitor()
{
    float heading = imu_getHeading();
    
    uint8_t sys, gyro, accel, mag;
    imu_getCalibration(sys, gyro, accel, mag);

    Serial.print("Local North Heading: ");
    Serial.print(heading, 2);
    Serial.print(" deg  |  Calib (S/G/A/M): ");
    
    Serial.print(sys); Serial.print("/");
    Serial.print(gyro); Serial.print("/");
    Serial.print(accel); Serial.print("/");
    Serial.println(mag);

    delay(100);
}

#endif