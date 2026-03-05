#ifndef LED_MASK_H
#define LED_MASK_H

#include <Arduino.h>
#include <stdint.h>
#include <math.h>

#define NUM_LEDS 64
#define LED_PHYSICAL_OFFSET -23 // Shift to match physical wiring of LED #0

// ==========================
// Mask Generation Logic
// ==========================

// Converts a standardized angle to a 64-bit shift register mask
inline uint64_t angleToLedMask(float angleDeg, int &ledIndexOut) 
{
    // Fast normalization to 0.0 - 359.9 using fmod
    angleDeg = fmod(angleDeg, 360.0f);
    if (angleDeg < 0) angleDeg += 360.0f;

    // Calculate logical LED index (add 0.5 for nearest-neighbor rounding)
    float ledsPerDegree = (float)NUM_LEDS / 360.0f;
    int logicalIndex = (int)(angleDeg * ledsPerDegree + 0.5f) % NUM_LEDS;
    
    // Safely handle negative offsets to prevent undefined bit-shifting
    int rawIndex = logicalIndex + LED_PHYSICAL_OFFSET;
    ledIndexOut = ((rawIndex % NUM_LEDS) + NUM_LEDS) % NUM_LEDS;
    
    // Set the single bit corresponding to the LED index
    return (1ULL << ledIndexOut);
}

// Generates the specific mask for North (Blinking)
// Generates the specific mask for North (Blinking)
inline uint64_t getNorthMask(float heading, bool isLedOn) 
{
    if (!isLedOn) return 0ULL; 

    // This is effectively (heading - 0)
    float northAngle = heading; 
    
    int northLedIndex;
    return angleToLedMask(northAngle, northLedIndex);
}

// Generates the specific mask for the Target (Solid)
inline uint64_t getTargetMask(float heading, float bearing) 
{
    // Reversed subtraction to account for your hardware's wiring direction
    float relativeAngle = heading - bearing;

    int targetLedIndex;
    return angleToLedMask(relativeAngle, targetLedIndex);
}

inline uint64_t getCheckerboardMask() {
    return 0xAAAAAAAAAAAAAAAAULL; 
}

// ==========================
// Debugging
// ==========================

inline void printMask(uint64_t mask) 
{
    Serial.print("MASK: 0x ");
    for (int i = 7; i >= 0; i--) {
        uint8_t b = (mask >> (i * 8)) & 0xFF;
        if (b < 0x10) Serial.print("0");
        Serial.print(b, HEX);
        Serial.print(" ");
    }
    Serial.println();
}

#endif // LED_MASK_H