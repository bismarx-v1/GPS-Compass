#ifndef GNSS_NMEA_H
#define GNSS_NMEA_H

#include <Arduino.h>
#include <ArduinoNmeaParser.h>
#include "GPIO_MAP.h"

/* ================= CURRENT POSITION ================= */

double compass_lat = 0.0;
double compass_lon = 0.0;

uint8_t compass_satellites_used     = 0;  // from GGA (used in fix)
uint8_t compass_satellites_in_view  = 0;  // from GSV (total visible)

/* ================= CONFIG ================= */

#define GNSS_BAUD 9600
extern HardwareSerial GNSS;

/* ================= FORWARD DECLARATIONS ================= */

void gnss_onRmcUpdate(nmea::RmcData const rmc);
void gnss_onGgaUpdate(nmea::GgaData const gga);

/* ================= PARSER INSTANCE ================= */
/* NOT static → must be accessible from main */

ArduinoNmeaParser gnss_parser(gnss_onRmcUpdate, gnss_onGgaUpdate);

/* ================= SETUP ================= */

inline void gnss_setup()

{   Serial.println("GNSS UART started");
    GNSS.begin(GNSS_BAUD, SERIAL_8N1, GNSS_RX, GNSS_TX);
}

/* ================= MANUAL GSV PARSER ================= */

inline void gnss_parse_gsv(char c)
{
    static String gsv_buffer;

    if (c == '\n') {

        if (gsv_buffer.startsWith("$GPGSV") ||
            gsv_buffer.startsWith("$GLGSV") ||
            gsv_buffer.startsWith("$GNGSV")) {

            int firstComma  = gsv_buffer.indexOf(',');
            int secondComma = gsv_buffer.indexOf(',', firstComma + 1);
            int thirdComma  = gsv_buffer.indexOf(',', secondComma + 1);

            if (thirdComma > 0) {
                String satsStr = gsv_buffer.substring(secondComma + 1, thirdComma);
                compass_satellites_in_view = (uint8_t)satsStr.toInt();
            }
        }

        gsv_buffer = "";
    }
    else if (c != '\r') {
        gsv_buffer += c;
    }
}

/* ================= CALLBACKS ================= */

inline void gnss_onRmcUpdate(nmea::RmcData const rmc)
{
    if (!rmc.is_valid) return;

    compass_lat = rmc.latitude;
    compass_lon = rmc.longitude;
}

inline void gnss_onGgaUpdate(nmea::GgaData const gga)
{
    compass_satellites_used = gga.num_satellites;
}

#endif
