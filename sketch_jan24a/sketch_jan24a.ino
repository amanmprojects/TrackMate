#include <WiFi.h>
#include <WebServer.h>
#include <RadioLib.h>

// LoRa Pins
#define SCK     18
#define MISO    19
#define MOSI    23
#define NSS     5
#define RESET   27
#define BUSY    26
#define DIO1    -1 // Not connected in most cases; set to -1 if unused

// LoRa Module
SX1262 lora = new Module(NSS, DIO1, RESET, BUSY, SPI);  // Pass SPI object directly

// WiFi Credentials
const char* ssid = "POCO F4";
const char* password = "1234567890";

// Web Server
WebServer server(80);

// Status Variables
String statusLog = "";

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Set up the web server
  server.on("/", handleRoot);
  server.begin();
  Serial.println("Web server started!");

  // Debug LoRa Initialization
  debugLoRa();
}

void loop() {
  server.handleClient();
}

void debugLoRa() {
  // Reset Module
  pinMode(RESET, OUTPUT);
  digitalWrite(RESET, LOW);
  delay(10);
  digitalWrite(RESET, HIGH);
  delay(10);

  // Initialize LoRa
  int state = lora.begin(868.0); // Frequency in MHz (adjust for your region)
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("LoRa initialized successfully!");
    statusLog += "LoRa initialized successfully!\n";
  } else {
    Serial.print("LoRa initialization failed, code: ");
    Serial.println(state);
    statusLog += "LoRa initialization failed, code: " + String(state) + "\n";
  }

  // Check BUSY Pin State
  pinMode(BUSY, INPUT);
  int busyState = digitalRead(BUSY);
  Serial.print("BUSY Pin State: ");
  Serial.println(busyState);
  statusLog += "BUSY Pin State: " + String(busyState) + "\n";

  // Test SPI Communication
  digitalWrite(NSS, LOW);
  byte response = SPI.transfer(0x42);  // Send dummy byte
  digitalWrite(NSS, HIGH);
  Serial.print("SPI Test Response: 0x");
  Serial.println(response, HEX);
  statusLog += "SPI Test Response: 0x" + String(response, HEX) + "\n";

  // Test Transmission
  state = lora.transmit("Test Message");
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("Transmission successful!");
    statusLog += "Transmission successful!\n";
  } else {
    Serial.print("Transmission failed, code: ");
    Serial.println(state);
    statusLog += "Transmission failed, code: " + String(state) + "\n";
  }
}

// Web Server Root Handler
void handleRoot() {
  String html = "<html><body>";
  html += "<h1>LoRa Debug Status</h1>";
  html += "<pre>" + statusLog + "</pre>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}
