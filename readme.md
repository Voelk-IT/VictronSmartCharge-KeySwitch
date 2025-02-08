# VictronSmartCharge-KeySwitch

## Description
VictronSmartCharge-KeySwitch is an ESP32-based solution designed to control the **Victron MultiPlus** inverter/charger using a key switch. The system detects when the generator is started and sends an MQTT signal to **Home Assistant** to control the **Cerbo GX** for intelligent charging. The ESP32 can be configured via a web-based Captive Portal and supports WiFi and MQTT connection setup.

## Features
- **WiFi and MQTT Setup** via Captive Portal
- **Key Switch (GPIO 21)** state detection to control MultiPlus charging
- Sends **MQTT messages** for key switch status, last update timestamp, and WiFi connection status
- **Automatic WiFi connection** and reconnection if the connection is lost
- **NTP time synchronization** for accurate timestamping
- Compatible with **Home Assistant** for automation
- **Preferences Storage** for saving configuration data

## Installation

### 1. Flashing the ESP32
- Use the **Arduino IDE** or **PlatformIO** to flash the ESP32 with the provided firmware.
- Make sure you have the required libraries installed:
  - `WiFi.h`
  - `WebServer.h`
  - `Preferences.h`
  - `PubSubClient.h`
  - `TimeLib.h`
  - `NTPClient.h`
  - `WiFiUdp.h`

### 2. Configuration
The ESP32 will start in **Access Point (AP) mode** if no WiFi or MQTT configuration is found. You can access the **Captive Portal** to set up WiFi and MQTT settings.

- **WiFi Setup**: Enter the SSID and password for your network.
- **MQTT Setup**: Enter the IP address, username, and password of your MQTT broker.

Once you save the settings, the device will restart and connect to the configured network.

### 3. MQTT Topics
- `esp32/schluesselschalter/status` - Reports the status of the key switch (0 = OFF, 1 = ON).
- `esp32/schluesselschalter/lastupdate` - Sends the last update timestamp.
- `esp32/schluesselschalter/wificonnect` - Sends the timestamp of WiFi connection.

### 4. Home Assistant Integration
You can use **Home Assistant** to automate actions based on the key switch status or other MQTT topics.

## Wiring
- Connect a **key switch** to **GPIO 21** for state detection.
- Ensure the ESP32 is powered with a stable 5V source.

## Future Improvements
- Add status feedback from the **Victron MultiPlus**.
- Extend support for additional generators.
- Add web interface for configuration without Captive Portal.

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgements
- **ESP32**: Microcontroller used for handling WiFi and MQTT communication.
- **PubSubClient**: MQTT client library.
- **Home Assistant**: For MQTT automation.
- **Victron Energy**: For the MultiPlus and Cerbo GX system.

## Contact
For support or questions, feel free to reach out via [GitHub Issues](https://github.com/Voelk-IT/VictronSmartCharge-KeySwitch/issues).
