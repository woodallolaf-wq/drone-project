#include <esp_now.h>
#include <WiFi.h>
#include "esp_wifi.h"
#include <math.h>

#define RSSI_CUTOFF      -53   // CALIBRATE THIS — see Section 4 of spec
#define ROLLING_WINDOW    10
#define KILL_BYTE        0xAA
#define MAX_RANGE_RSSI   -80   // ignore if drone beyond 25m
#define RSSI_1M          -40   // CALIBRATE: measured RSSI at exactly 1 metre
#define PATH_LOSS_EXP     2.0f
#define CONN_TIMEOUT_MS   500
#define PRINT_INTERVAL_MS 250

int rssiBuffer[ROLLING_WINDOW];
int bufIndex = 0;
bool bufferFull = false;
bool killSent = false;
volatile int lastRSSI = -99;
volatile unsigned long lastPacketTime = 0;
volatile float estimatedDistance = 0.0f;
volatile int espnowCount = 0;

uint8_t droneMAC[] = {0x58, 0x8C, 0x81, 0xAB, 0x2E, 0xC0}; // Unit A (drone) MAC

void IRAM_ATTR promiscuousCb(void *buf, wifi_promiscuous_pkt_type_t type) {
  wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;
  lastRSSI = pkt->rx_ctrl.rssi;
}

void onReceive(const uint8_t *mac, const uint8_t *data, int len) {
  if (killSent) return;

  espnowCount++;
  int rssi = lastRSSI;
  lastPacketTime = millis();

  if (rssi < MAX_RANGE_RSSI) return;

  rssiBuffer[bufIndex] = rssi;
  bufIndex = (bufIndex + 1) % ROLLING_WINDOW;
  if (bufIndex == 0) bufferFull = true;

  int count = bufferFull ? ROLLING_WINDOW : bufIndex;
  if (count == 0) return;

  int sum = 0;
  for (int i = 0; i < count; i++) sum += rssiBuffer[i];
  int avgRSSI = sum / count;

  estimatedDistance = pow(10.0f, (RSSI_1M - avgRSSI) / (10.0f * PATH_LOSS_EXP));

  if (avgRSSI > RSSI_CUTOFF) {
    uint8_t killMsg = KILL_BYTE;
    for (int i = 0; i < 3; i++) {
      esp_now_send(droneMAC, &killMsg, 1);
      delay(10);
    }
    killSent = true;
  }
}

void setup() {
  Serial.begin(115200);
  delay(3000);
  WiFi.mode(WIFI_STA);

  Serial.print("This device MAC: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ERROR: esp_now_init failed");
    while (true) delay(1000);
  }
  Serial.println("ESP-NOW init OK");

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, droneMAC, 6);
  peer.channel = 0;
  peer.encrypt = false;
  esp_now_add_peer(&peer);

  esp_now_register_recv_cb(onReceive);

  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(promiscuousCb);

  Serial.println("Kill-switch receiver ready.");
}

unsigned long lastPrint = 0;

void loop() {
  if (millis() - lastPrint < PRINT_INTERVAL_MS) return;
  lastPrint = millis();

  if (killSent) {
    Serial.println("STATUS: KILL SENT — motor cut");
    return;
  }

  bool connected = (millis() - lastPacketTime) < CONN_TIMEOUT_MS;

  if (!connected) {
    Serial.println("STATUS: Drone not detected");
  } else {
    Serial.print("STATUS: Connected | RSSI: ");
    Serial.print(lastRSSI);
    Serial.print(" dBm | Distance: ~");
    Serial.print(estimatedDistance, 1);
    Serial.println(" m");
  }
}
