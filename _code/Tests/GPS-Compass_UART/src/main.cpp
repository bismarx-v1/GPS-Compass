#include <Arduino.h>

#define UART0_RX 44
#define UART0_TX 43

HardwareSerial GNSS(0);  // Use UART0 for GNSS module

void setup() {
  Serial.begin(115200);
  delay(500);

  GNSS.begin(
    9600,                // Baud rate (adjust if needed)
    SERIAL_8N1,
    UART0_RX,
    UART0_TX
  );

  Serial.println("UART0 listener started");
}

void loop() {
  while (GNSS.available()) {
    uint8_t c = GNSS.read();
    Serial.write(c);   // Forward raw UART data to USB
  }
}
