#include <Arduino.h>
#include <ArduinoNmeaParser.h>

#define GNSS_RX 44
#define GNSS_TX 43

// Forward declarations
void onRmcUpdate(nmea::RmcData const rmc);
void onGgaUpdate(nmea::GgaData const gga);

// Parser instance
ArduinoNmeaParser parser(onRmcUpdate, onGgaUpdate);

void setup()
{
  Serial.begin(115200);

  Serial1.begin(
    9600,          // GNSS baud rate
    SERIAL_8N1,
    GNSS_RX,       // RX = GPIO44
    GNSS_TX        // TX = GPIO43
  );
}

void loop()
{
  while (Serial1.available()) {
    parser.encode((char)Serial1.read());
  }
}

/* ================= CALLBACKS ================= */

void onRmcUpdate(nmea::RmcData const rmc)
{
  Serial.print("RMC ");

  Serial.print(rmc.time_utc.hour);
  Serial.print(":");
  Serial.print(rmc.time_utc.minute);
  Serial.print(":");
  Serial.print(rmc.time_utc.second);

  Serial.print(rmc.is_valid ? " A " : " V ");

  if (rmc.is_valid) {
    Serial.print("Lat=");
    Serial.print(rmc.latitude, 6);
    Serial.print(" Lon=");
    Serial.print(rmc.longitude, 6);
    Serial.print(" Speed=");
    Serial.print(rmc.speed);
    Serial.print(" Course=");
    Serial.print(rmc.course);
  }

  Serial.println();
}

void onGgaUpdate(nmea::GgaData const gga)
{
  Serial.print("GGA ");

  Serial.print(gga.time_utc.hour);
  Serial.print(":");
  Serial.print(gga.time_utc.minute);
  Serial.print(":");
  Serial.print(gga.time_utc.second);

  if (gga.fix_quality != nmea::FixQuality::Invalid) {
    Serial.print(" Fix=");
    Serial.print((int)gga.fix_quality);
    Serial.print(" Sats=");
    Serial.print(gga.num_satellites);
    Serial.print(" HDOP=");
    Serial.print(gga.hdop);
    Serial.print(" Alt=");
    Serial.print(gga.altitude);
    Serial.print("m");
  }

  Serial.println();
}
