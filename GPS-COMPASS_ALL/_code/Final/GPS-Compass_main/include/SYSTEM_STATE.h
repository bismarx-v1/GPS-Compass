#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

struct HardwareData {
    float vbat = 0;
    float ichg = 0;
    float temp = 0;
    bool webServerEnabled = false;
    bool lastButtonState = HIGH; // Moved here
};

struct UIState {
    unsigned long lastNorthToggle = 0;
    const unsigned long NORTH_BLINK_INTERVAL = 500;
    bool northLedState = true;
};

struct Position {
    float lat = 0;
    float lon = 0;
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
extern UIState ui;

#endif