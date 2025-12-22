#include <Arduino.h>

#define TP 10
#define LED 37
#define SRCLK 10
#define SER 12 
#define RCLK 11


// Send out one bit.
void shift_bit(bool bit) {
  digitalWrite(SER, bit);
  delayMicroseconds(1); 
  digitalWrite(SRCLK,1);
  delayMicroseconds(1); 
  digitalWrite(SRCLK,0);
  delayMicroseconds(1);
}

// Send out one bit eight times (one byte).
void shift_byte(char chomp) {
  for(char bit_index = 0; bit_index < 8; bit_index++) {
    shift_bit((chomp >> bit_index) & 1);
  }
}

// Send out one byte eight times (64 bits).
void shift_8byte(long long nom) {
  for(long long byte_index = 0; byte_index < 8; byte_index++){
    // shift_byte((char)((nom >> (byte_index * 8)) & 0b11111111));
    shift_byte(nom >> (byte_index * 8));
  }
  
}

void triggerBuffers() {
  digitalWrite(RCLK,1);
  delayMicroseconds(1); 
  digitalWrite(RCLK,0);
  delayMicroseconds(1);
}

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(SRCLK, OUTPUT);
  pinMode(SER, OUTPUT);
  pinMode(RCLK, OUTPUT);
  pinMode(TP, OUTPUT);
  digitalWrite(TP, HIGH);
  Serial.begin(115200);
  Serial.print("Start: ");
  Serial.println(micros());
  shift_8byte(0xf035000000000000);  // Send 64 bits.
  // shift_byte(0b00110101);
  // shift_byte(202);
  
  
  triggerBuffers();
  Serial.print("End: ");
  Serial.println(micros());
  Serial.print("End2: ");
  Serial.println(micros());
  
}

void loop() {
  static unsigned long long displayVal = 0;
  digitalWrite(LED, HIGH);
  shift_8byte(0xf035000000000000);
  triggerBuffers();

  if(displayVal < (unsigned long long)-1) {
    displayVal++;
  } else {
    displayVal = 0;
  }

  digitalWrite(LED, LOW);
  delay(1000);
}
