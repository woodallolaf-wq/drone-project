# Drone Project

## ESP32-C3 Setup & MAC Address Retrieval

### Hardware
- Board: ESP32-C3 (Seeed XIAO ESP32-C3 or similar)
- Connected via USB on COM5

### Prerequisites
- [PlatformIO](https://platformio.org/) installed (VS Code extension)

### Project Structure
```
drone project/
├── src/
│   └── main.cpp       # Main firmware source
├── platformio.ini     # PlatformIO build config
├── .env               # Device-specific values (not committed)
└── .gitignore
```

### platformio.ini Configuration
The board requires the following settings for USB CDC serial output to work:

```ini
[env:esp32dev]
platform = espressif32
board = esp32-c3-devkitm-1
framework = arduino
monitor_speed = 115200
build_flags =
    -DARDUINO_USB_MODE=1
    -DARDUINO_USB_CDC_ON_BOOT=1
```

> Without `-DARDUINO_USB_CDC_ON_BOOT=1`, `Serial` output will not appear over USB on the ESP32-C3.

### Retrieving the MAC Address

The firmware in `src/main.cpp` reads and prints the device MAC address over serial:

```cpp
#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(3000); // Wait for serial monitor to connect
  WiFi.mode(WIFI_STA);
  Serial.println(WiFi.macAddress());
}

void loop() {}
```

**Steps:**
1. Upload firmware: `Ctrl+Alt+U`
2. Open serial monitor: `Ctrl+Alt+S`
3. Press **RESET** on the board
4. MAC address prints after ~3 seconds

### Device Info
| Key | Value |
|-----|-------|
| MAC Address | See `.env` → `ESP32_MAC` |
| Baud Rate | 115200 |
| Port | COM5 |
