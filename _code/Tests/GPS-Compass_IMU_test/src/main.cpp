#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

#define SDA_PIN 6 
#define SCL_PIN 7 

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire); 

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN, 100000); 
  delay(500); 

  if (!bno.begin()) {
    Serial.println("BNO055 not found!");
    while (1);
  }
}

void loop() {
  // Get Euler angles (in degrees)
  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  
  // X = Yaw (Heading), Y = Roll, Z = Pitch
  Serial.print("Euler: ");
  Serial.print(euler.x(), 2);
  Serial.print(", ");
  Serial.print(euler.y(), 2);
  Serial.print(", ");
  Serial.println(euler.z(), 2);

  delay(20); 
}

//cd C:\Users\MenMe\Documents\GitHub\GPS-Compass\_code\Python_IMU_viz