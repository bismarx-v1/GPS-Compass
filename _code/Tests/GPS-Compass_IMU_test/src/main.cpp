#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

#define SDA_PIN 6 
#define SCL_PIN 7 

// Explicitly pass &Wire to guarantee it uses your custom pins
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire); 

void setup() {
  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN, 100000); 
  delay(3000); 

  if (!bno.begin()) {
    Serial.println("BNO055 not found!");
    while (1);
  }
}

void loop() {
  // Get Quaternion data directly
  imu::Quaternion quat = bno.getQuat();
  
  // Format exactly as the Python script expects: "Quaternion: w, x, y, z"
  Serial.print("Quaternion: ");
  Serial.print(quat.w(), 4);
  Serial.print(", ");
  Serial.print(quat.x(), 4);
  Serial.print(", ");
  Serial.print(quat.y(), 4);
  Serial.print(", ");
  Serial.println(quat.z(), 4);

  // 20ms delay gives a smooth 50 FPS for 3D rendering
  delay(20); 
}

//cd C:\Users\MenMe\Documents\GitHub\GPS-Compass\_code\Python_IMU_viz
//IMU.py