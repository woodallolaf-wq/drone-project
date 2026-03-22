#include <esp_now.h>
#include <WiFi.h>

#define KILL_PIN         21    // GPIO21 = D7 on XIAO ESP32C3 silkscreen
#define KILL_BYTE        0xAA
#define BEACON_INTERVAL  100   // ms

bool killed = false;
unsigned long lastBeacon = 0;
uint8_t groundMAC[] = {0x58, 0x8C, 0x81, 0xAE, 0xBE, 0x64}; // Unit B MAC

void onReceive(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (data[0] == KILL_BYTE && !killed) {
    killed = true;
    digitalWrite(KILL_PIN, HIGH); // MOSFET gate HIGH = motor power cut
    // LATCHED: nothing sets KILL_PIN LOW again except power cycle
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  pinMode(KILL_PIN, OUTPUT);
  digitalWrite(KILL_PIN, LOW); // MOSFET off = motor runs at startup

  esp_now_init();

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, groundMAC, 6);
  peer.channel = 0;
  peer.encrypt = false;
  esp_now_add_peer(&peer);

  esp_now_register_recv_cb(onReceive);
}

void loop() {
  if (!killed && millis() - lastBeacon >= BEACON_INTERVAL) {
    uint8_t beacon = 0x01;
    esp_now_send(groundMAC, &beacon, 1);
    lastBeacon = millis();
  }
}
