#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>


#define WIFI_SSID "POCO F4"
#define WIFI_PASSWORD "1234567890"
#define API_KEY "AIzaSyDAThZT3V7ugWNuWl0UWObQaFo07oiGv00"
#define DATABASE_URL "https://trackmate-vcet-default-rtdb.firebaseio.com/"


FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long  sendDataPrevMillis = 0;
bool signupOK = false;

/************** Server Configuration **************/
String serverIP = "192.168.189.140";  // IP address of the first ESP32
String serverPath = "/gps";           // Path set on the first ESP32

void setup(){
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); 
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if(Firebase.signUp(&config, &auth, "", "")){
    Serial.println("Firebase auth successfull");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}


void loop(){
  if (Firebase.ready() && signupOK){
    StaticJsonDocument<256> gpsData;
    if (getGPSData(gpsData)) {
      // gpsData structure will contain "valid" field and, if valid,
      // the rest of the coordinates from the first ESP32 [1].

      // Check if valid
      bool valid = gpsData["valid"];
      if (!valid) {
        Serial.println("GPS not valid yet, skipping Firebase update...\n");
      }
      else {
        double latitude   = gpsData["latitude"];
        double longitude  = gpsData["longitude"];
        double altitude   = gpsData["altitude"];
        double hdop       = gpsData["hdop"];
        int satellites    = gpsData["satellites"];
        String timestamp  = gpsData["timestamp"].as<String>();

        Serial.println("Got valid GPS data:");
        Serial.print("  Latitude: ");  Serial.println(latitude, 6);
        Serial.print("  Longitude: "); Serial.println(longitude, 6);
        Serial.print("  Timestamp: "); Serial.println(timestamp);
        Serial.println("Uploading to Firebase...\n");

        // Upload to Firebase RTDB
        // Example path: /locations/Dog_1/...
        // This matches the schema from your question [1].
        String path = "/locations/Dog_1";

        // lastSeen as epoch time (if desired) or store the timestamp string directly
        // For a quick example, we'll just store the Arduino "millis" as lastSeen:
        Firebase.RTDB.setInt(&fbdo, path + "/lastSeen", millis());

        // Store latitude/longitude
        Firebase.RTDB.setDouble(&fbdo, path + "/latitude", latitude);
        Firebase.RTDB.setDouble(&fbdo, path + "/longitude", longitude);

        // Optionally, store altitude, hdop, satellites, etc.
        // e.g. Firebase.RTDB.setDouble(&fbdo, path + "/hdop", hdop);
      }
    }
    else {
      Serial.println("Failed to retrieve GPS data.\n");
    }

  // Delay before the next attempt
  delay(5000);
  }
}




/************** Function to GET GPS data from the first ESP32 **************/
bool getGPSData(StaticJsonDocument<256> &doc) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "http://" + serverIP + serverPath;
    http.begin(url);
    
    int httpCode = http.GET();
    if (httpCode == 200) { // OK
      String payload = http.getString();
      DeserializationError error = deserializeJson(doc, payload);
      http.end();

      if (error) {
        Serial.print("JSON parsing failed: ");
        Serial.println(error.f_str());
        return false;
      }
      return true;
    } else {
      Serial.print("HTTP GET failed, error code: ");
      Serial.println(httpCode);
      http.end();
      return false;
    }
  } else {
    Serial.println("WiFi not connected!");
    return false;
  }
}