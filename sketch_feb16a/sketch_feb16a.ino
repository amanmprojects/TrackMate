#include <WiFi.h>
#include <WebServer.h>

// Replace with your network credentials
const char* ssid = "POCO F4";
const char* password = "1234567890";

WebServer server(80);

void setup() {
  Serial.begin(115200);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, []() {
    server.send(200, "text/plain", "Hello, world!");
  });
  
  server.begin();
}

void loop() {
  server.handleClient();
  delay(2);
}
