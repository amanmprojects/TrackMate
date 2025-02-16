#include <RadioLib.h>

// Pin definitions for ESP32
#define NSS     5    // SPI Chip Select
#define MOSI   23    // SPI MOSI
#define MISO   19    // SPI MISO
#define SCK    18    // SPI Clock
#define RX_EN  21    // RX Enable
#define TX_EN   4    // TX Enable
#define BUSY   22    // SX1262 Busy pin
#define DIO1   33    // DIO1 pin
#define RESET  14    // Reset pin

// Create an instance of the SX1262 class
SX1262 radio = new Module(NSS, DIO1, RESET, BUSY);
// SX126x::XTAL = false;

void setup() {
  // Initialize Serial for debugging
  Serial.begin(9600);
  while (!Serial);

  // Set RF switch control pins
  // radio.setRfSwitchPins(TX_EN, RX_EN);
  radio.setDio2AsRfSwitch(false);  // Disable DIO2 RF switch control


  // Initialize the radio with frequency and set tcxoVoltage to 0 for XTAL
// Disable TCXO (set last parameter to 0)
int state = radio.begin(865.0, 125.0, 7, 5, 0, 8, 0); 

  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("SX1262 successfully initialized!"));
  } else {
    Serial.print(F("Failed to initialize SX1262, code "));
    Serial.println(state);
    while (true);
  }
}

void loop() {
  // Your main code here
}
