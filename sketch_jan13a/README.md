# ESP32 GPS Module Reader

This version of the code implements a basic GPS coordinate reader using an ESP32 microcontroller and GPS module.

## Components
- ESP32
- GPS module (connected via Serial2)
- TinyGPS++ library

## How it works
The program continually reads GPS data through Serial2 and prints:
- Latitude (6 decimal precision)
- Longitude (6 decimal precision)
- Altitude (in meters)
- Number of satellites

## Configuration
- GPS Baud Rate: 9600
- RX Pin: 16 
- TX Pin: 17
- Serial Monitor Baud Rate: 115200

## Output Format
When GPS location is valid:
```
Latitude: XX.XXXXXX
Longitude: YY.YYYYYY
Altitude: ZZ.ZZ meters
Satellites: N
```

When GPS signal is not available:
```
Location: INVALID
```