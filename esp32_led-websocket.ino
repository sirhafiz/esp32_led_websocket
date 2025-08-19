#include <WiFi.h>
#include <WiFiManager.h> // Untuk konfigurasi WiFi
#include <WebSocketsServer.h> // Untuk WebSocket
#include <ESPAsyncWebServer.h> // Untuk pelayan web asinkron
#include <FS.h>
#include <SPIFFS.h>

#define LED_PIN 13 // Pin LED (ubah jika perlu)

AsyncWebServer server(80); // Pelayan web pada port 80
WebSocketsServer webSocket = WebSocketsServer(81); // WebSocket pada port 81

void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("WebSocket [%u] Terputus!\n", num);
      break;
    case WStype_CONNECTED:
      Serial.printf("WebSocket [%u] Bersambung dari %s\n", num, webSocket.remoteIP(num).toString().c_str());
      break;
    case WStype_TEXT:
      Serial.printf("WebSocket [%u] Mesej: %s\n", num, payload);
      if (strcmp((char*)payload, "ON") == 0) {
        digitalWrite(LED_PIN, HIGH);
        Serial.println("LED ON");
        webSocket.sendTXT(num, "Status LED: ON");
      } else if (strcmp((char*)payload, "OFF") == 0) {
        digitalWrite(LED_PIN, LOW);
        Serial.println("LED OFF");
        webSocket.sendTXT(num, "Status LED: OFF");
      }
      break;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // Matikan LED pada mulanya
  
  // Mulakan SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("Gagal mulakan SPIFFS");
    while (1) delay(100);
  }
  
  // WiFi Manager dengan AP tanpa kata laluan
  WiFiManager wm;
  wm.setConfigPortalTimeout(120); // Masa tamat AP 120 saat
  if (!wm.autoConnect("ESP32_LED_AP", NULL)) { // NULL untuk tiada kata laluan
    Serial.println("Gagal sambung WiFi, restart...");
    ESP.restart();
  }
  Serial.println("Sambungan WiFi berjaya!");
  Serial.print("Alamat IP: ");
  Serial.println(WiFi.localIP());
  
  // Pelayan web untuk menghantar index.html
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });
  
  server.begin();
  
  // Mulakan WebSocket
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
}

void loop() {
  webSocket.loop(); // Kendali WebSocket
}