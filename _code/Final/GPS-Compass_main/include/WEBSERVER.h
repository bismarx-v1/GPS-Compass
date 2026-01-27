#include <WiFi.h>
#include <Arduino.h>
#include "webpage.h"
#include "esp_system.h"
#include "SHARED_TYPES.h"

//WiFi Access Point credentials
const char* ssid     = "GPS-Compass";
const char* password = "bismarx123";

float lat, lon;

WiFiServer server(80);

const int LED_PIN = 2;

void web_setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); //sets the LED off on boot
  WiFi.softAP(ssid, password); //this thingy creates the access point

  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP()); //this thingy prints IP adress

  server.begin();
}

void parseCoords(const char* request, float& lat, float& lon) {
  
    lat = 0.0f;
    lon = 0.0f;

    const char* latPointer = strstr(request, "lat=");
    const char* lonPointer = strstr(request, "lon=");
    if (latPointer) {
        latPointer += 4;                // skip "lat="
        lat = strtof(latPointer, nullptr);
    }

    if (lonPointer) {
        lonPointer += 4;                // skip "lon="
        lon = strtof(lonPointer, nullptr);
    }
}

enum webRetErr {
    OKI,
    EWWOW,
};

struct web_ret {
    Position pos;
    webRetErr err;
};

web_ret web_loop() {
  web_ret ret;
  WiFiClient client = server.available();

  ret.err = EWWOW;
  if (!client) return ret;

  String request = "";

  // Read first line from butter.
  while (client.connected()) {
    if (client.available()) {
      char c = client.read();
      request += c;
      if (c == '\n') break;   // First line is enough
    }
  }


  // Only parse and print coordinates when the /coords endpoint is requested
  if (request.indexOf("GET /coords?") >= 0) {
    parseCoords(request.c_str(), lat, lon);
    // Serial.printf("Received Coordinates:\n"
    //               "Latitude: %4.d\n"
    //               "Longitude: %4.d\n",
    //               lat, lon
    // );
ret.pos.lat = lat;
ret.pos.lon = lon;

    Serial.println("Received Coordinates:");
    Serial.print("Latitude: ");
    Serial.println(lat, 4);

    Serial.print("Longitude: ");
    Serial.println(lon, 4);
ret.err = OKI;
  }
  /* ===================== HTTP RESPONSE ===================== */
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html; charset=UTF-8");
  client.println("Connection: close");
  client.println();
  client.println(webpage);


  client.stop();
  return ret;
}
