#ifndef SHIFT_REG_H
#define SHIFT_REG_H

#include <Arduino.h>
#include <Wire.h>
#include "GPIO_MAP.h"

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

void triggerBuffers() { //sends data to the output pins
  digitalWrite(LATCH_SR, HIGH);
  delayMicroseconds(1); 
  digitalWrite(LATCH_SR, LOW);
  delayMicroseconds(1);
}

#endif // SHIFT_REG_H
