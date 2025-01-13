#include <TinyGPS++.h>

#define GPS_BAUDRATE 9600
#define RX_PIN 16
#define TX_PIN 17

TinyGPSPlus gps;

void setup() {
  Serial.begin(115200);
  Serial2.begin(GPS_BAUDRATE, SERIAL_8N1, RX_PIN, TX_PIN);
  
  Serial.println("ESP32 GPS Example");
}

void loop() {
  while (Serial2.available() > 0) {
    if (gps.encode(Serial2.read())) {
      if (gps.location.isValid()) {
        Serial.print("Latitude: ");
        Serial.println(gps.location.lat(), 6);
        Serial.print("Longitude: ");
        Serial.println(gps.location.lng(), 6);
        Serial.print("Altitude: ");
        Serial.println(gps.altitude.meters());
      } else {
        Serial.println("Location: Not Available");
      }
      
      Serial.print("Satellites: ");
      Serial.println(gps.satellites.value());
      Serial.println();
    }
    // Serial.println(Serial2.read());
  }


}
