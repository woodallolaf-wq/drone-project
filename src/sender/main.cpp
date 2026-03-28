#include <esp_now.h>
#include <WiFi.h>

#define KILL_PIN         10    // GPIO21 = D7 on XIAO ESP32C3 silkscreen
#define KILL_BYTE        0xAA
#define BEACON_INTERVAL  100   // ms

bool killed = false;
unsigned long lastBeacon = 0;
uint8_t groundMAC[] = {0x58, 0x8C, 0x81, 0xAE, 0xBE, 0x64}; // Unit B (ground) MAC

void onReceive(const uint8_t *mac, const uint8_t *data, int len) {
  if (data[0] == KILL_BYTE && !killed) {
    killed = true;
    digitalWrite(KILL_PIN, LOW); // MOSFET gate LOW = motor power cut
    // LATCHED: nothing sets KILL_PIN HIGH again except power cycle
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  pinMode(KILL_PIN, OUTPUT);
  digitalWrite(KILL_PIN, HIGH); // MOSFET on = motor runs at startup

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
