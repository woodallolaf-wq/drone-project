#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(3000);
  WiFi.mode(WIFI_STA);
  Serial.println(WiFi.macAddress());
}

void loop() {}