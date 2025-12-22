#include <WiFi.h>
#include <Arduino.h>
#include "webpage.h"

const char* ssid     = "GPS-Compass";
const char* password = "bismarx123";

WiFiServer server(80);

const int LED_PIN = 2;

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); //sets the LED off on boot

  WiFi.softAP(ssid, password); //this thingy creates the access point

  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP()); //this thingy prints IP adress

  server.begin();
}

void loop() {
  WiFiClient client = server.available();

  if (!client) return;

  String request = "";

  while (client.connected()) {
    if (client.available()) {
      char c = client.read();
      request += c;
      if (c == '\n') break;   // First line is enough
    }
  }

  /* ===================== LED CONTROL ===================== */
  if (request.indexOf("GET /led?state=1") >= 0) {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("LED ON");
  }
  if (request.indexOf("GET /led?state=0") >= 0) {
    digitalWrite(LED_PIN, LOW);
    Serial.println("LED OFF");
  }

  /* ===================== HTTP RESPONSE ===================== */
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println(webpage);

  client.stop();
}
