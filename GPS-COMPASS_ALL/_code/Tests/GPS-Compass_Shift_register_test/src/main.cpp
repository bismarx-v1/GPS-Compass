#include <Arduino.h>
#define CLK_SR 40 //SCK
#define SI_SR 39 //SER
#define LATCH_SR 21 //RCK

long long mask = 0;

// Send out one bit.
void shift_bit(bool bit) {
  digitalWrite(SI_SR, bit);
  delayMicroseconds(1); 
  digitalWrite(CLK_SR,1);
  delayMicroseconds(1); 
  digitalWrite(CLK_SR,0);
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
    shift_byte(nom >> (byte_index * 8));
  }
}

void triggerBuffers() {
  digitalWrite(LATCH_SR,1);
  delayMicroseconds(1); 
  digitalWrite(LATCH_SR,0);
  delayMicroseconds(1);
}

void setup() {
  pinMode(CLK_SR, OUTPUT);
  pinMode(SI_SR, OUTPUT);
  pinMode(LATCH_SR, OUTPUT);
  Serial.begin(115200);
  Serial.print("Start: ");
  Serial.println(micros());
  shift_8byte(mask);  // Send 64 bits.
  
  triggerBuffers();
  Serial.print("End: ");
  Serial.println(micros());
  Serial.print("End2: ");
  Serial.println(micros());
  
}

void loop() {
  static unsigned long long displayVal = 0;
  shift_8byte(mask);
  triggerBuffers();

  if(displayVal < (unsigned long long)-1) {
    displayVal++;
  } else {
    displayVal = 0;
  }
}
