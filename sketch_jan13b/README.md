# GPS Tracking System with ESP32 and Python

This version of the code implements a GPS tracking system using an ESP32 microcontroller with a GPS module and a Python client.

## Components
- ESP32 with GPS module
- Python client with mapping capabilities

## How it works

### ESP32 (sketch_jan13b.ino)
- Connects to WiFi network
- Creates a webserver
- Reads GPS data from the connected GPS module
- Exposes GPS coordinates via a REST API endpoint at `/gps`
- Returns location data in JSON format using ArduinoJson library

### Python Client (test.py)
- Makes periodic GET requests to the ESP32's API endpoint
- Parses the received JSON GPS coordinates
- Uses Folium library to plot coordinates on OpenStreetMap
- Generates and updates "cat_location.html" with the latest position
- Prints coordinates and satellite count to console

## Testing
Successfully tested using mobile hotspot connection between ESP32 and Python client.

## Requirements
- ESP32 libraries: TinyGPS++, WiFi, WebServer, ArduinoJson
- Python libraries: requests, folium