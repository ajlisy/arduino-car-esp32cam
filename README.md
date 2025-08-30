# ESP32-CAM Arduino Car Project

This project contains Arduino code for an ESP32-CAM based car with camera streaming capabilities.

## Hardware Requirements

- ESP32-CAM module
- Car chassis with motors
- Motor driver board
- Power supply (3.3V for ESP32, appropriate voltage for motors)

## Setup Instructions

### 1. WiFi Configuration

Before uploading the code, you need to configure your WiFi credentials in the `examples/camerawebserver.ino` file:

```cpp
// Replace with your network credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
```

**⚠️ Important: Never commit your actual WiFi credentials to the repository!**

### 2. Arduino IDE Setup

1. Install Arduino IDE
2. Add ESP32 board support:
   - Go to File > Preferences
   - Add this URL to "Additional Board Manager URLs": `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Go to Tools > Board > Boards Manager
   - Search for "ESP32" and install "ESP32 by Espressif Systems"

3. Select the correct board:
   - Tools > Board > ESP32 Arduino > AI Thinker ESP32-CAM

4. Install required libraries:
   - Tools > Manage Libraries
   - Search for and install "ESP32 Camera"

### 3. Uploading the Code

1. Connect your ESP32-CAM to your computer via USB
2. Select the correct COM port
3. Upload the `camerawebserver.ino` sketch
4. Open the Serial Monitor (115200 baud) to see connection status

### 4. Accessing the Camera Stream

Once connected to WiFi, the ESP32-CAM will print its IP address to the Serial Monitor. You can access the camera stream by opening a web browser and navigating to:

```
http://[ESP32_IP_ADDRESS]
```

## Features

- Real-time camera streaming
- WiFi connectivity
- Camera configuration options
- PSRAM support for better performance

## Pin Configuration

The code uses the standard ESP32-CAM pin configuration:

- Camera pins are automatically configured
- WiFi uses the built-in WiFi module
- Serial communication for debugging

## Troubleshooting

- **Camera not working**: Check if the camera module is properly connected
- **WiFi connection issues**: Verify your SSID and password
- **Poor image quality**: Adjust the JPEG quality settings in the code
- **Memory issues**: Ensure PSRAM is enabled if available

## Contributing

Feel free to submit issues and enhancement requests!

## License

This project is open source. Please respect the license terms.
