/*
 * CG2271 Assignment 4 — ESP32 side
 *
 * Responsibilities:
 *   1. Connect to Wi-Fi (credentials in secrets.h — NOT committed to Git)
 *   2. Receive packets from the MCXC444 over UART (Serial2)
 *   3. Call the cloud service with that data
 *   4. Send actuator commands back to the MCXC444
 *   5. (TODO) Read the ESP32-side sensor / drive the ESP32-side actuator
 *
 * Based on as4_sample.ino (c) Low Tjun Lym, NUS — see docs/as4_sample_original.ino.
 *
 * Libraries: WiFi.h, HTTPClient.h (built in), Arduino_JSON (Library Manager).
 */
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

#include "secrets.h"    // copy secrets.example.h -> secrets.h and fill in
#include "protocol.h"   // copied from common/ by scripts/sync_protocol.sh

// ---- UART link to MCXC444 ---------------------------------------------------
#define MCX_RX_PIN 16   // ESP32 RX  <- MCXC444 TX
#define MCX_TX_PIN 17   // ESP32 TX  -> MCXC444 RX
HardwareSerial &mcx = Serial2;
PktParser parser;

// ---- Cloud call timing (respect API rate limits) ----------------------------
const unsigned long CLOUD_PERIOD_MS = 10000;
unsigned long lastCloudCall = 0;

// Latest data received from the MCXC444
Packet latestSensor;
bool   haveSensorData = false;

// -----------------------------------------------------------------------------
void sendPacket(uint8_t type, const uint8_t *payload, uint8_t len) {
  uint8_t buf[PKT_MAX_SIZE];
  size_t n = pkt_encode(buf, type, payload, len);
  if (n) mcx.write(buf, n);
}

void sendAck(uint8_t type, bool ok) {
  uint8_t p = type;
  sendPacket(ok ? MSG_ACK : MSG_NACK, &p, 1);
}

void handlePacket(const Packet &pkt) {
  switch (pkt.type) {
    case MSG_SENSOR_DATA:
      latestSensor = pkt;
      haveSensorData = true;
      sendAck(pkt.type, true);
      break;
    case MSG_EVENT:
      Serial.printf("[MCX] event id=%u\n", pkt.len ? pkt.payload[0] : 0);
      // TODO(team): react immediately, e.g. trigger a cloud call / notification
      sendAck(pkt.type, true);
      break;
    case MSG_ACK:
    case MSG_NACK:
      Serial.printf("[MCX] %s for 0x%02X\n", pkt.type == MSG_ACK ? "ACK" : "NACK",
                    pkt.len ? pkt.payload[0] : 0);
      break;
    case MSG_PING:
      sendPacket(MSG_PING, nullptr, 0);
      break;
    default:
      sendAck(pkt.type, false);
  }
}

void pollUart() {
  while (mcx.available()) {
    int r = pkt_parse_byte(&parser, (uint8_t)mcx.read());
    if (r == 1) handlePacket(parser.pkt);
    else if (r == -1) Serial.println("[MCX] bad packet dropped");
  }
}

String httpGET(const String &url) {
  WiFiClient client;
  HTTPClient http;
  http.begin(client, url);
  int code = http.GET();
  String body = "{}";
  if (code > 0) body = http.getString();
  else Serial.printf("[HTTP] error %d\n", code);
  http.end();
  return body;
}

// TODO(team): replace this example with your real online function
// (database write, OpenAI / Gemini call, web app, MQTT, ...).
void doCloudCall() {
  String url = "http://api.openweathermap.org/data/2.5/weather?q=Singapore,SG&APPID=" +
               String(OPENWEATHER_API_KEY);
  JSONVar obj = JSON.parse(httpGET(url));
  if (JSON.typeof(obj) == "undefined") {
    Serial.println("[HTTP] JSON parse failed");
    return;
  }
  int humidity = (int)obj["main"]["humidity"];
  Serial.printf("[CLOUD] humidity=%d\n", humidity);

  // Example: send a command back to the MCXC444
  uint8_t cmd[2] = { 0 /* actuator id */, (uint8_t)humidity };
  sendPacket(MSG_ACTUATOR_CMD, cmd, sizeof(cmd));
}

// -----------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  mcx.begin(AS4_UART_BAUD, SERIAL_8N1, MCX_RX_PIN, MCX_TX_PIN);
  pkt_parser_init(&parser);

  Serial.printf("Connecting to %s", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.print("\nConnected, IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  pollUart();   // never block here — keep the UART drained

  if (millis() - lastCloudCall > CLOUD_PERIOD_MS) {
    lastCloudCall = millis();
    if (WiFi.status() == WL_CONNECTED) doCloudCall();
    else Serial.println("WiFi disconnected");
  }
}
