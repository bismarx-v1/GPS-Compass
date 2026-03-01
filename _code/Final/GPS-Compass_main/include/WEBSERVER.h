#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <WiFi.h>
#include "webpage.h"
#include "SYSTEM_STATE.h" 
#include "EEPROM.h" 
extern void enterLightSleep(); // <-- ADD THIS: Links to the function in MAIN

const char* ssid     = "GPS-Compass";
const char* password = "bismarx123";  

WiFiServer server(80);

void web_setup(bool enable) {
    if (enable) {
        WiFi.mode(WIFI_AP); // Ensure mode is set before starting
        WiFi.softAP(ssid, password);
        server.begin();
        Serial.println("WiFi AP Started");
    } else {
        server.stop();
        WiFi.softAPdisconnect(true); 
        WiFi.mode(WIFI_OFF); // <--- This kills the radio hardware
        Serial.println("WiFi AP Disconnected & Radio OFF");
    }
}

void parseCoords(const char* request, float& lat, float& lon) {
  const char* latPtr = strstr(request, "lat=");
  const char* lonPtr = strstr(request, "lon=");
  lat = latPtr ? strtof(latPtr + 4, nullptr) : 0.0f;
  lon = lonPtr ? strtof(lonPtr + 4, nullptr) : 0.0f;
}

void web_loop() {
    WiFiClient client = server.available();
    if (!client) return;

    client.setTimeout(10); 
    String request = client.readStringUntil('\r');
    client.flush();

 if (request.indexOf("GET /coords?") >= 0) {
        parseCoords(request.c_str(), target_position.lat, target_position.lon);
        
        // --- ADD THESE LINES ---
        // Save to EEPROM (Float is 4 bytes, so addresses are 0x00 and 0x04)
        eepromWrite(0x0000, target_position.lat);
        eepromWrite(0x0004, target_position.lon);
        // -----------------------
        
        client.print("HTTP/1.1 200 OK\r\n\r\nOK");
    }
    else if (request.indexOf("GET /battery") >= 0) {
        float percent = constrain(((sys.vbat - 3.3f) / (4.2f - 3.3f)) * 100.0f, 0, 100);
        
        String json = "{ \"voltage\":" + String(sys.vbat, 2) + 
                      ", \"percentage\":" + String(percent, 0) + 
                      ", \"temperature\":" + String(sys.temp, 1) + " }";

        client.print("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n" + json);
    }
    else if (request.indexOf("GET /sleep") >= 0) { // <-- ADD THIS BLOCK
        client.print("HTTP/1.1 200 OK\r\n\r\nSystem going to sleep.");
        client.stop(); // Force network close before shutting down
        delay(100); 
        enterLightSleep();
    }
    else {
        client.print("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" + String(webpage));
    }
    client.stop();
}

#endif