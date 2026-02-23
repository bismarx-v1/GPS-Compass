#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include <Arduino.h>

struct Position {
    float lat = 0.0f;
    float lon = 0.0f;
};

struct HardwareData {
    float vbat = 0;
    float ichg = 0;
    float temp = 0;
    bool webServerEnabled = false;
};

struct MonitorState {
    char mode = 'e';
    unsigned long lastPrint = 0;
    const unsigned long INTERVAL = 3000;
};

// Declarations (Promises to the compiler)
extern HardwareData sys;
extern Position target_position;
extern MonitorState mon;

#endif