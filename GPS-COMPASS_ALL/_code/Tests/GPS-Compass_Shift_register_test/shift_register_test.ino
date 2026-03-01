#include <Arduino.h>

#define SRCLK 10
#define SER 12 
#define RCLK 11

void shift_bit(bool bit) {
  digitalWrite(SER, bit);
  delay(1); 
  digitalWrite(SRCLK,1);
  delay(1); 
  digitalWrite(SRCLK,0);
  delay(1);
}

void setup() {
  pinMode(SRCLK, OUTPUT);
  pinMode(SER, OUTPUT);
  pinMode(RCLK, OUTPUT);
  delay(2000);
  shift_bit(1);
  shift_bit(0);

  
  digitalWrite(RCLK,1);
  delay(1); 
  digitalWrite(RCLK,0);
  delay(1);

}

void loop() {
  delay(100);
}
