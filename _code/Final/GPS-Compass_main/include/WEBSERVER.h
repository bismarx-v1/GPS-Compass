#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <WiFi.h>
#include <Arduino.h>
#include "webpage.h"
#include "CHARGER.h"
#include "SHARED_TYPES.h"

// WiFi Access Point credentials
const char* ssid     = "GPS-Compass";
const char* password = "bismarx123";  

extern float g_vbat;
extern float g_ichg;

/* ================= TARGET POSITION ================= */

Position target_position = {0.0f, 0.0f};

WiFiServer server(80);

/* ===================== SETUP ===================== */
void web_setup() {
  WiFi.softAP(ssid, password);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  server.begin();
}

/* ===================== PARSE COORDS ===================== */
void parseCoords(const char* request, float& lat, float& lon) {
  lat = 0.0f;
  lon = 0.0f;

  const char* latPointer = strstr(request, "lat=");
  const char* lonPointer = strstr(request, "lon=");

  if (latPointer) {
    latPointer += 4;
    lat = strtof(latPointer, nullptr);
  }

  if (lonPointer) {
    lonPointer += 4;
    lon = strtof(lonPointer, nullptr);
  }
}

/* ===================== MAIN WEB LOOP ===================== */

Position web_loop() {

  Position pos;
  pos.lat = 0.0f;
  pos.lon = 0.0f;

WiFiClient client = server.available();
if (!client) return pos;

client.setTimeout(2);
String request = client.readStringUntil('\r');

if (request.length() == 0) {
  client.stop();
  return pos;
}

if (request.length() == 0) {
  client.stop();
  return pos;
}

  /* ===================== COORDS ENDPOINT ===================== */
  if (request.indexOf("GET /coords?") >= 0) {

    float lat = 0.0f;
    float lon = 0.0f;

    parseCoords(request.c_str(), lat, lon);

    target_position.lat = lat;
    target_position.lon = lon;

    Serial.println("Received Coordinates:");
    Serial.print("Latitude: ");
    Serial.println(lat, 4);
    Serial.print("Longitude: ");
    Serial.println(lon, 4);

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();
    client.println("OK");

    client.stop();
    return pos;
}

  /* ===================== BATTERY ENDPOINT ===================== */
  if (request.indexOf("GET /battery") >= 0)
{
    float percent = (g_vbat / 4.2f) * 100.0f;
    percent = constrain(percent, 0.0f, 100.0f);

    String json = "{";
    json += "\"voltage\":" + String(g_vbat, 2) + ",";
    json += "\"percentage\":" + String(percent, 0) + ",";
    json += "\"current\":" + String(g_ichg, 0);
    json += "}";


    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();
    client.println(json);

    client.stop();
    return pos;
  }

  /* ===================== DEFAULT: SERVE WEBPAGE ===================== */
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html; charset=UTF-8");
  client.println("Connection: close");
  client.println();
  client.println(webpage);

  client.stop();
  return pos;
}

#endif