# Drone Project

## ESP32-C3 Setup & ESP-NOW Communication

### Hardware
- Board: Seeed Studio XIAO ESP32C3
- Connected via USB on COM5

### Prerequisites
- [PlatformIO](https://platformio.org/) installed (VS Code extension)

### Project Structure
```
drone project/
├── src/
│   ├── main.cpp            # MAC address retrieval utility
│   ├── receiver/main.cpp   # ESP-NOW receiver firmware
│   └── sender/main.cpp     # ESP-NOW sender firmware
├── platformio.ini          # PlatformIO build config (receiver + sender envs)
├── .env                    # Device-specific values (not committed)
└── .gitignore
```

### platformio.ini Configuration
Two environments are defined — one per role. USB CDC flags are required for `Serial` output over USB on the ESP32-C3.

```ini
[env:receiver]
platform = espressif32
board = seeed_xiao_esp32c3
framework = arduino
monitor_speed = 115200
build_flags =
    -DARDUINO_USB_MODE=1
    -DARDUINO_USB_CDC_ON_BOOT=1
build_src_filter = -<*> +<receiver/>

[env:sender]
platform = espressif32
board = seeed_xiao_esp32c3
framework = arduino
monitor_speed = 115200
build_flags =
    -DARDUINO_USB_MODE=1
    -DARDUINO_USB_CDC_ON_BOOT=1
build_src_filter = -<*> +<sender/>
```

> Without `-DARDUINO_USB_CDC_ON_BOOT=1`, `Serial` output will not appear over USB on the ESP32-C3.

---

## Step 1 — Get Receiver MAC Address

Flash `src/main.cpp` to the receiver board to retrieve its MAC address.

```cpp
#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(3000);
  WiFi.mode(WIFI_STA);
  Serial.println(WiFi.macAddress());
}

void loop() {}
```

**Steps:**
1. Upload firmware: `Ctrl+Alt+U`
2. Open serial monitor: `Ctrl+Alt+S`
3. Press **RESET** — MAC address prints after ~3 seconds
4. Save the MAC to `.env` as `ESP32_MAC`

---

## Step 2 — Flash Receiver

Select the `receiver` environment in the PlatformIO toolbar, then upload to the receiving board. It listens for incoming ESP-NOW messages and prints them to serial.

---

## Step 3 — Flash Sender

Select the `sender` environment, update `receiverMAC[]` in `src/sender/main.cpp` with the receiver's MAC, then upload to the sending board. It transmits `"hello"` every second.

**Expected output:**
- Receiver serial: `Received: hello`
- Sender serial: `Delivered`

---

## Why ESP-NOW (not Bluetooth)

The ESP32-C3 only supports BLE — no Classic Bluetooth. ESP-NOW is the better choice:
- No router required — peer-to-peer directly between boards
- Lower latency than WiFi or BLE
- No pairing process

---

## Device Info
| Key | Value |
|-----|-------|
| MAC Address | See `.env` → `ESP32_MAC` |
| Baud Rate | 115200 |
| Port | COM5 |
