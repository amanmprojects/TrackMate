#include <SPI.h>
#include <RadioLib.h>

// Create instance of SX1262 radio
SX1262 radio = new Module(5,     // NSS pin
                         13,    // DIO1 pin
                         12,    // RESET pin
                         14);   // BUSY pin

// Function to read IRQ status
void readIrqStatus() {
    digitalWrite(5, LOW);
    delayMicroseconds(100);
    SPI.transfer(0x12);  // GetIrqStatus command
    uint8_t irqH = SPI.transfer(0x00);
    uint8_t irqL = SPI.transfer(0x00);
    delayMicroseconds(100);
    digitalWrite(5, HIGH);
    
    Serial.print("IRQ Status: 0x");
    Serial.print(irqH, HEX);
    Serial.println(irqL, HEX);
}

// Function to read status register
void readStatus(const char* label = "Status") {
    digitalWrite(5, LOW);
    delayMicroseconds(100);
    uint8_t status = SPI.transfer(0xC0);
    uint8_t response = SPI.transfer(0x00);
    delayMicroseconds(100);
    digitalWrite(5, HIGH);
    
    Serial.print(label);
    Serial.print(" - Status: 0x");
    Serial.print(status, HEX);
    Serial.print(", Response: 0x");
    Serial.println(response, HEX);
}

bool sendCommand(uint8_t cmd, uint8_t* data, uint8_t length, const char* cmdName) {
    Serial.print("\nSending command ");
    Serial.print(cmdName);
    Serial.print(" (0x");
    Serial.print(cmd, HEX);
    Serial.println(")");
    
    readStatus("Before command");
    
    unsigned long startWait = millis();
    while(digitalRead(14) == HIGH) {
        if(millis() - startWait > 100) {
            Serial.println("BUSY stuck HIGH!");
            return false;
        }
        delay(1);
    }
    
    digitalWrite(5, LOW);
    delayMicroseconds(100);
    SPI.transfer(cmd);
    for(uint8_t i = 0; i < length; i++) {
        SPI.transfer(data[i]);
    }
    delayMicroseconds(100);
    digitalWrite(5, HIGH);
    delay(10);
    
    readStatus("After command");
    return true;
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n--- SX1262 Radio Test (IRQ Monitoring) ---");
    
    // Initialize pins
    pinMode(5, OUTPUT);   // NSS
    pinMode(23, OUTPUT);  // MOSI
    pinMode(19, INPUT);   // MISO
    pinMode(18, OUTPUT);  // SCK
    pinMode(14, INPUT);   // BUSY
    pinMode(12, OUTPUT);  // RESET
    pinMode(13, INPUT);   // DIO1
    
    digitalWrite(5, HIGH);
    
    // Initialize SPI
    SPI.begin(18, 19, 23, 5);
    SPI.beginTransaction(SPISettings(32000, MSBFIRST, SPI_MODE0));
    
    // Reset sequence
    Serial.println("Performing reset...");
    digitalWrite(12, HIGH);
    delay(20);
    digitalWrite(12, LOW);
    delay(50);
    digitalWrite(12, HIGH);
    delay(100);
    
    // Configuration sequence
    uint8_t data;
    
    // Set to standby mode
    data = 0x00;
    if(!sendCommand(0x80, &data, 1, "SetStandby")) return;
    delay(100);
    
    // Set packet type to LoRa
    data = 0x01;
    if(!sendCommand(0x8A, &data, 1, "SetPacketType")) return;
    delay(100);
    
    // Set frequency to 868 MHz
    uint32_t freq = (uint32_t)(868000000 / 0.95367431640625);
    uint8_t freqData[] = {
        (uint8_t)((freq >> 24) & 0xFF),
        (uint8_t)((freq >> 16) & 0xFF),
        (uint8_t)((freq >> 8) & 0xFF),
        (uint8_t)(freq & 0xFF)
    };
    if(!sendCommand(0x86, freqData, 4, "SetRfFrequency")) return;
    delay(100);
    
    // Set PA config
    uint8_t paConfigData[] = {0x02, 0x02, 0x00, 0x01};
    if(!sendCommand(0x95, paConfigData, 4, "SetPaConfig")) return;
    delay(100);
    
    // Set sync word
    uint8_t syncWord[] = {0x12, 0x34};
    if(!sendCommand(0x98, syncWord, 2, "SetLoRaSyncWord")) return;
    delay(100);
    
    // Configure DIO1 for TxDone
    uint8_t dioParams[] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    if(!sendCommand(0x08, dioParams, 8, "SetDioIrqParams")) return;
    delay(100);
    
    // Clear IRQ flags
    uint8_t clearIrq[] = {0xFF, 0xFF};
    if(!sendCommand(0x02, clearIrq, 2, "ClearIrqStatus")) return;
    delay(100);
    
    Serial.println("Radio initialized!");
}

void loop() {
    static unsigned long lastTransmit = 0;
    static uint8_t counter = 0;
    
    if (millis() - lastTransmit >= 5000) {
        Serial.println("\nPreparing new transmission...");
        
        // Clear IRQ flags
        uint8_t clearIrq[] = {0xFF, 0xFF};
        sendCommand(0x02, clearIrq, 2, "ClearIrqStatus");
        
        // Read current IRQ status
        Serial.println("IRQ status before transmission:");
        readIrqStatus();
        
        // Write test data to buffer
        digitalWrite(5, LOW);
        delayMicroseconds(100);
        SPI.transfer(0x0E);  // WriteBuffer
        SPI.transfer(0x00);  // Buffer address
        for(uint8_t i = 0; i < 8; i++) {
            SPI.transfer(i + counter);  // Test pattern
        }
        delayMicroseconds(100);
        digitalWrite(5, HIGH);
        delay(10);
        
        // Start transmission
        uint8_t txData[] = {0x00, 0x00, 0x00};  // No timeout
        if(sendCommand(0x83, txData, 3, "SetTx")) {
            Serial.println("Transmission started");
            
            unsigned long startWait = millis();
            bool txComplete = false;
            
            while(millis() - startWait < 2000) {  // 2 second timeout
                if((millis() - startWait) % 200 == 0) {  // Check every 200ms
                    readStatus("During TX");
                    readIrqStatus();
                }
                
                if(digitalRead(13) == HIGH) {  // DIO1 indicates TX done
                    txComplete = true;
                    break;
                }
            }
            
            if(txComplete) {
                Serial.println("Transmission completed successfully!");
            } else {
                Serial.println("Transmission failed - timeout");
                
                // Try to recover
                digitalWrite(12, LOW);  // Reset
                delay(10);
                digitalWrite(12, HIGH);
                delay(100);
            }
        }
        
        // Return to standby
        uint8_t standbyData = 0x00;
        sendCommand(0x80, &standbyData, 1, "SetStandby");
        delay(100);
        
        counter++;
        lastTransmit = millis();
    }
    
    if(Serial.available()) {
        char cmd = Serial.read();
        if(cmd == 's') {
            readStatus("Manual check");
            readIrqStatus();
        } else if(cmd == 'r') {
            Serial.println("\nRestarting...");
            setup();
        }
    }
}