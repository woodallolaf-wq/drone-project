# Drone Project

## ESP32-C3 Proximity Kill-Switch

Two XIAO ESP32C3 boards communicate over ESP-NOW. The **receiver** (Unit B, carried by the person) monitors signal strength from the drone. When the drone gets too close, it sends a kill command. The **sender** (Unit A, on the drone) beacons continuously and cuts motor power when it receives that command.

### Hardware
- Board: Seeed Studio XIAO ESP32C3
- Connected via USB on COM5

### Prerequisites
- [PlatformIO](https://platformio.org/) installed (VS Code extension)

### Project Structure
```
drone project/
├── src/
│   ├── receiver/main.cpp   # Unit B — worn by person, monitors RSSI, sends kill
│   └── sender/main.cpp     # Unit A — on drone, beacons and cuts motors on kill
├── platformio.ini          # PlatformIO build config (receiver + sender envs)
└── .gitignore
```

### platformio.ini Configuration

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

## Unit A — Sender (on drone)

**MAC:** `58:8C:81:AB:2E:C0`

**Behaviour:**
- On boot: sets GPIO10 `HIGH` (MOSFET on, motor runs)
- Every 100 ms: sends a `0x01` beacon to the receiver via ESP-NOW
- On receiving `0xAA` (kill byte): sets GPIO10 `LOW` (MOSFET off, motor cut), latched — only clears on power cycle

**Upload command:**
```
C:\Users\Wooda\.platformio\penv\Scripts\platformio.exe run -e sender -t upload
```

---

## Unit B — Receiver (carried by person)

**MAC:** `58:8C:81:AE:BE:64`

**Behaviour:**
- Enables WiFi promiscuous mode to capture raw RSSI from every packet (more reliable than the ESP-NOW callback RSSI)
- On each beacon received: adds RSSI to a rolling window of 10 samples, computes average
- Estimates distance using the log-distance path loss model: `d = 10 ^ ((RSSI_1M − avgRSSI) / (10 × 2.0))`
- If `avgRSSI > -53 dBm` (drone too close): sends `0xAA` kill byte to drone 3 times, latches `killSent`
- Ignores packets with RSSI below `-80 dBm` (drone out of range, >~25 m)
- Prints status to serial every 250 ms

**Serial output examples:**
```
STATUS: Drone not detected
STATUS: Connected | RSSI: -61 dBm | Distance: ~3.5 m
STATUS: KILL SENT — motor cut
```

**Upload command:**
```
C:\Users\Wooda\.platformio\penv\Scripts\platformio.exe run -e receiver -t upload
```

---

## Calibration

| Parameter | Define | Default | Notes |
|-----------|--------|---------|-------|
| Kill threshold | `RSSI_CUTOFF` | `-53` | RSSI above this triggers kill. Measure at desired kill distance. |
| Reference RSSI | `RSSI_1M` | `-40` | Measured RSSI at exactly 1 m. Board-specific. |
| Max tracking range | `MAX_RANGE_RSSI` | `-80` | Packets weaker than this are ignored (~25 m+). |
| Path loss exponent | `PATH_LOSS_EXP` | `2.0` | 2.0 = free space. Increase for obstructed environments. |

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
| Baud Rate | 115200 |
| Port | COM5 |
| Unit A MAC (drone) | `58:8C:81:AB:2E:C0` |
| Unit B MAC (person) | `58:8C:81:AE:BE:64` |
