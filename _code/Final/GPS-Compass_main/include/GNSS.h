#ifndef _GNSS_H
#define _GNSS_H



#include <Arduino.h>
#include "GPIO_MAP.h"
#include "SHARED_TYPES.h"



char gnss_buffer[gnss_buffer_size];
const uint16_t gnss_buffer_size = 676;

HardwareSerial Serial_GNSS(2);
void gnss_setup() {

    Serial_GNSS.begin(
        9600,                // Baud rate (adjust if needed)
        SERIAL_8N1,
        GNSS_TX,
        GNSS_RX
    );

    Serial.setRxBufferSize(gnss_buffer_size * 3);
}




int8_t parseSign(char sign) {
    switch(sign) {
    case 'N':
        return 1;
    case 'S':
        return -1;
    case 'E':
        return 1;   // Idunno.
    case 'W':
        return -1;  // Idunno.
    
    default:
        return 0;
    }
}


Position gnss_parseData(const char* data) {
    Position parsedPosition;
    uint16_t idx, fieldIdx;
    bool loopKeper = true;
    idx = fieldIdx = 0;

    while(loopKeper) {
        if(idx == UINT16_MAX) {
            loopKeper = false;
            continue;
        }
        
        switch(data[idx]) {  
        case ',':
            fieldIdx++;
            break;
        case '$':
            loopKeper = false;
            continue;
        default:
            idx++;
            continue;
        }

        switch(fieldIdx) {
        case 1: // Latitude.
            parsedPosition.lat = atof(data + idx);
            break;
        case 2: // Latitude sign.
            parsedPosition.lat *= parseSign(data[idx]);
            break;
        case 3: // Longitude.
            parsedPosition.lon = atof(data + idx);
            break;
        case 4: // Longitude sign.
            parsedPosition.lon *= parseSign(data[idx]);
            break;
        case 5:
            return parsedPosition;
        
        default:
            break;
        }
    }

    return parsedPosition;
}


enum GnssError {
    OKI,
    EWWOW,
};

struct GnssRetStruct {
    GnssError error;
    Position position;
};

GnssRetStruct gnss_update() {
    GnssRetStruct gnssReturn;
    gnssReturn.error = EWWOW;

    if(Serial_GNSS.available() < gnss_buffer_size) {
        return gnssReturn;
    }

    Serial_GNSS.readBytes(gnss_buffer, gnss_buffer_size);
    char* rowPtr = strstr(gnss_buffer, "$GPGGA,");
    if(Serial_GNSS.available() > gnss_buffer_size * 2) {
        Serial_GNSS.flush();
    }

    gnssReturn.error = OKI;
    gnssReturn.position = gnss_parseData(rowPtr);
    return gnssReturn; 
}


#endif
