#include <TinyGPS++.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define GPS_BAUDRATE 9600
#define RX_PIN 18
#define TX_PIN 19

TinyGPSPlus gps;
WebServer server(80);

// WiFi Configuration
const char* ssid = "POCO F4";
const char* password = "1234567890";

// Firebase
#define API_KEY "AIzaSyDAThZT3V7ugWNuWl0UWObQaFo07oiGv00"
#define DATABASE_URL "https://trackmate-vcet-default-rtdb.firebaseio.com/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

// GPS Data Structure
struct GPSData {
  double lat = 19.295626480587067;
  double lng = 72.87439413152563;
  double alt = 0.0;
  double hdop = 0.0;
  int satellites = 0;
  String timestamp = "";
  bool valid = false;
} currentGPS;

void setup() {
  Serial.begin(115200);
  Serial2.begin(GPS_BAUDRATE, SERIAL_8N1, RX_PIN, TX_PIN);

  connectWiFi();
  
  server.on("/gps", handleGPS);
  server.begin();
  Serial.println("HTTP server started");

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("Firebase auth successful");
    signupOK = true;
  }
  else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback; // Provide the tokenStatusCallback function
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  server.handleClient();
  readGPS();
}

void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void readGPS() {
  bool newData = false;
  while (Serial2.available() > 0) {
    if (gps.encode(Serial2.read())) {
      updateGPSData();
      updateFirebase();
      newData = true;
      Serial.println("GPS data received");
    }
  }
  if (!newData) {
    // Use dummy if no data was read
    // currentGPS = getDummyGPSData();
    Serial.println("GPS NAHi MIla");
  }
}

void updateGPSData() {
  currentGPS.valid = false;
  
  if (gps.location.isValid() && gps.date.isValid() && gps.time.isValid()) {
    currentGPS.lat = gps.location.lat();
    currentGPS.lng = gps.location.lng(); // Corrected field name
    currentGPS.alt = gps.altitude.meters();
    currentGPS.hdop = gps.hdop.hdop();
    currentGPS.satellites = gps.satellites.value();
    
    char timestamp[25];
    snprintf(timestamp, sizeof(timestamp), "%04d-%02d-%02dT%02d:%02d:%02dZ",
             gps.date.year(), gps.date.month(), gps.date.day(),
             gps.time.hour(), gps.time.minute(), gps.time.second());
    currentGPS.timestamp = String(timestamp);
    currentGPS.valid = true;
  }
  else {
    Serial.println("GPS data not valid");
  }
}

void handleGPS() {
  StaticJsonDocument<256> doc;
  doc["valid"] = currentGPS.valid;
  
  if (currentGPS.valid) {
    doc["latitude"] = currentGPS.lat;
    doc["longitude"] = currentGPS.lng;
    doc["altitude"] = currentGPS.alt;
    doc["hdop"] = currentGPS.hdop;
    doc["satellites"] = currentGPS.satellites;
    doc["timestamp"] = currentGPS.timestamp;
  } else {
    doc["message"] = "Awaiting GPS fix";
    doc["satellites"] = gps.satellites.value();
  }

  String jsonResponse;
  serializeJson(doc, jsonResponse);
  server.send(200, "application/json", jsonResponse);
}

GPSData getDummyGPSData() {
  // Declare a static GPSData structure that persists between calls
  static GPSData dummy;
  
  // Initialize once or keep incrementing
  static double baseLat = 19.295626;
  static double baseLng = 72.874394;
  static double baseAlt = 10.0; // Just an arbitrary altitude

  // Slightly modify the base values each time the function is called
  baseLat += 0.000001;
  baseLng -= 0.000001;
  baseAlt += 0.1;

  // Populate the dummy structure
  dummy.lat = baseLat;
  dummy.lng = baseLng;
  dummy.alt = baseAlt;
  dummy.hdop = 0.9; // Arbitrary HDOP
  dummy.satellites = 5; // Arbitrary satellite count
  dummy.valid = true; // Mark as valid since it’s dummy data

  // Generate a simple timestamp as if it’s changing
  char timestamp[25];
  static int seconds = 0; // Increment seconds for a changing time
  seconds++;
  snprintf(timestamp, sizeof(timestamp), "2023-01-01T00:%02d:%02dZ", seconds / 60, seconds % 60);
  dummy.timestamp = String(timestamp);

  return dummy;
}

void updateFirebase() {
  if (Firebase.ready() && signupOK) {
    StaticJsonDocument<256> gpsData;

    // Check if valid
    bool valid = currentGPS.valid;
    if (!valid) {
      Serial.println("GPS not valid yet, skipping Firebase update...\n");
    }
    else {
      double latitude = currentGPS.lat;
      double longitude = currentGPS.lng; // Corrected field name
      double altitude = currentGPS.alt;
      double hdop = currentGPS.hdop;
      int satellites = currentGPS.satellites;
      String timestamp = currentGPS.timestamp;

      Serial.println("Got valid GPS data:");
      Serial.print("  Latitude: ");  Serial.println(latitude, 6);
      Serial.print("  Longitude: "); Serial.println(longitude, 6);
      Serial.print("  Timestamp: "); Serial.println(timestamp);
      Serial.println("Uploading to Firebase...\n");

      // Upload to Firebase RTDB
      String path = "/locations/Dog_1";

      Firebase.RTDB.setInt(&fbdo, path + "/lastSeen", millis());
      Firebase.RTDB.setDouble(&fbdo, path + "/latitude", latitude);
      Firebase.RTDB.setDouble(&fbdo, path + "/longitude", longitude);
      Firebase.RTDB.setDouble(&fbdo, path + "/altitude", altitude);
      Firebase.RTDB.setDouble(&fbdo, path + "/hdop", hdop);
      Firebase.RTDB.setInt(&fbdo, path + "/satellites", satellites);
      Firebase.RTDB.setString(&fbdo, path + "/timestamp", timestamp);
    }
  }
}