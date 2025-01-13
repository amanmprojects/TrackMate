#include <TinyGPS++.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

#define GPS_BAUDRATE 9600
#define RX_PIN 16
#define TX_PIN 17

TinyGPSPlus gps;

// Replace with your network credentials
const char* ssid = "POCO F4";
const char* password = "1234567890";

WebServer server(80); // Create a WebServer on port 80

void handleGPS() {
  StaticJsonDocument<200> doc;
  if (gps.location.isValid()) {
    doc["latitude"] = gps.location.lat();
    doc["longitude"] = gps.location.lng();
    doc["altitude"] = gps.altitude.meters();
    doc["satellites"] = gps.satellites.value();
  } else {
    doc["error"] = "Location not available";
  }

  String jsonString;
  serializeJson(doc, jsonString);

  server.send(200, "application/json", jsonString);
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(GPS_BAUDRATE, SERIAL_8N1, RX_PIN, TX_PIN);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/gps", handleGPS);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();

  // Read GPS data
  while (Serial2.available() > 0) {
    gps.encode(Serial2.read());
  }
}
