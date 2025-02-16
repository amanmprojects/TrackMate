#include <esp_system.h>

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("ESP32 is starting up...");
}

void loop() {
    if (Serial.available() > 0) {
        char input = Serial.read();
        if (input == 'r') {
            Serial.println("Performing ESP32 reset...");
            delay(1000);
            ESP.restart(); // Software reset the ESP32
        }
    }
}