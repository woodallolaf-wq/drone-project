#include <esp_now.h>
#include <WiFi.h>

void onReceive(const uint8_t *mac, const uint8_t *data, int len) {
  Serial.print("Received: ");
  Serial.println((char*)data);
}

void setup() {
  Serial.begin(115200);
  delay(3000);
  WiFi.mode(WIFI_STA);
  esp_now_init();
  esp_now_register_recv_cb(onReceive);
}

void loop() {}
