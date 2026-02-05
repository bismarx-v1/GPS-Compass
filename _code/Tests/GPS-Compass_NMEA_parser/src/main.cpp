#include <Arduino.h>
#include <ArduinoNmeaParser.h>

#define GNSS_RX 44
#define GNSS_TX 43

void onRmcUpdate(nmea::RmcData const);
void onGgaUpdate(nmea::GgaData const);

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
void onRmcUpdate(nmea::RmcData const rmc)
{
  Serial.print("RMC ");

  if      (rmc.source == nmea::RmcSource::GPS)     Serial.print("GPS");
  else if (rmc.source == nmea::RmcSource::GLONASS) Serial.print("GLONASS");
  else if (rmc.source == nmea::RmcSource::Galileo) Serial.print("Galileo");
  else if (rmc.source == nmea::RmcSource::GNSS)    Serial.print("GNSS");
  else if (rmc.source == nmea::RmcSource::BDS)     Serial.print("BDS");

  Serial.print(" ");
  Serial.print(rmc.time_utc.hour);
  Serial.print(":");
  Serial.print(rmc.time_utc.minute);
  Serial.print(":");
  Serial.print(rmc.time_utc.second);
  Serial.print(".");
  Serial.print(rmc.time_utc.microsecond);
  Serial.print(rmc.is_valid ? "A" : "V");
  Serial.print(" Lat:");
  Serial.print(rmc.latitude, 6);
  Serial.print(" Lon:");
  Serial.print(rmc.longitude, 6);
  Serial.print(" Speed:");
  Serial.print(rmc.speed, 2);
  Serial.print(" Course:");
  Serial.print(rmc.course, 2);
  Serial.print(" MagVar:");
  Serial.print(rmc.magnetic_variation, 2);
  Serial.print(" Date:");
  Serial.print(rmc.date.day);
  Serial.print("/");
  Serial.print(rmc.date.month);
  Serial.print("/");
  Serial.print(rmc.date.year);
  Serial.println();

}