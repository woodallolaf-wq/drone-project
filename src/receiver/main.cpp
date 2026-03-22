#include <esp_now.h>
#include <WiFi.h>

#define RSSI_CUTOFF    -54  // CALIBRATE THIS — see Section 4 of spec
#define ROLLING_WINDOW  10
#define KILL_BYTE      0xAA
#define MAX_RANGE_RSSI -80  // ignore if drone beyond 25m

int rssiBuffer[ROLLING_WINDOW];
int bufIndex = 0;
bool bufferFull = false;
bool killSent = false; // latch — never reset in software

uint8_t droneMAC[] = {0x__, 0x__, 0x__, 0x__, 0x__, 0x__}; // fill in Unit A MAC

void onReceive(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (killSent) return; // latch: stop all processing after kill

  int rssi = info->rx_ctrl->rssi;
  if (rssi < MAX_RANGE_RSSI) return; // drone out of tracking range

  // Rolling average
  rssiBuffer[bufIndex] = rssi;
  bufIndex = (bufIndex + 1) % ROLLING_WINDOW;
  if (bufIndex == 0) bufferFull = true;

  int count = bufferFull ? ROLLING_WINDOW : bufIndex;
  if (count == 0) return;

  int sum = 0;
  for (int i = 0; i < count; i++) sum += rssiBuffer[i];
  int avgRSSI = sum / count;

  if (avgRSSI > RSSI_CUTOFF) {
    // Triple-send for reliability (ESP-NOW has no ACK)
    uint8_t killMsg = KILL_BYTE;
    for (int i = 0; i < 3; i++) {
      esp_now_send(droneMAC, &killMsg, 1);
      delay(10);
    }
    killSent = true; // latch engaged
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  esp_now_init();

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, droneMAC, 6);
  peer.channel = 0;
  peer.encrypt = false;
  esp_now_add_peer(&peer);

  esp_now_register_recv_cb(onReceive);
}

void loop() {}
