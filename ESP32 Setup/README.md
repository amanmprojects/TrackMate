# ESP32 Setup Guide for Arduino IDE on Windows

## Prerequisites
1. Arduino IDE installed on your Windows system
2. USB cable for ESP32 connection

## Installation Steps

### 1. Install USB to UART Bridge Driver
- Download and install the CP210x USB to UART Bridge driver for ESP32 communication

### 2. Configure Arduino IDE
1. Open Arduino IDE
2. Go to `File > Preferences`
3. Add ESP32 board manager URL:
    ```
    https://dl.espressif.com/dl/package_esp32_index.json
    ```
4. Go to `Tools > Board > Boards Manager`
5. Search for "ESP32" and install the ESP32 board package by espressif systems

## Additional Resources
For detailed step-by-step instructions, watch this tutorial:
[ESP32 Setup Tutorial](https://youtu.be/DjfjECHtLg4)

## Note
Make sure your ESP32 board is properly connected and recognized by Windows before attempting to upload sketches.