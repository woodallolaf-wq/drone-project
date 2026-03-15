#include <esp_now.h>
#include <WiFi.h>

uint8_t receiverMAC[] = {0x58, 0x8C, 0x81, 0xAE, 0xBE, 0x64};

void onSent(const uint8_t *mac, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivered" : "Failed");
}

void setup() {
  Serial.begin(115200);
  delay(3000);
  WiFi.mode(WIFI_STA);
  esp_now_init();
  esp_now_register_send_cb(onSent);

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, receiverMAC, 6);
  peer.channel = 0;
  peer.encrypt = false;
  esp_now_add_peer(&peer);
}

void loop() {
  const char* msg = "hello";
  esp_now_send(receiverMAC, (uint8_t*)msg, strlen(msg) + 1);
  delay(1000);
}
