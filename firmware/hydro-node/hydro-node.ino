#include "secrets.h"

#include <WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

const int ONE_WIRE_BUS = 4;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

const char* MQTT_BROKER = "192.168.0.249";
const int MQTT_PORT = 1883;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void setup() {
  Serial.begin(115200);
  sensors.begin();
  delay(10000);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Connected!");

  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());

  // Raspberry Pi MQTT broker
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);

  Serial.print("Connecting to MQTT...");

  if (mqttClient.connect("hydro_01")) {
    Serial.println("connected!");
  } else {
    Serial.print("failed, state=");
    Serial.println(mqttClient.state());
  }
}

void loop() {

  if (WiFi.status() == WL_CONNECTED) {
      reconnectMQTT();
  }

  mqttClient.loop();
  sensors.requestTemperatures();

  float waterTemperature =
    sensors.getTempCByIndex(0);

  if (mqttClient.connected()) {

    String json =
      "{\"node\":\"hydro_01\","
      "\"readings\":{"
      "\"water_temperature\":"
      + String(waterTemperature, 2) + "}}";

    bool published = mqttClient.publish(
      "garden/hydro_01/telemetry",
      json.c_str());

    Serial.print("Sending: ");
    Serial.println(json);
    Serial.print("MQTT published: ");
    Serial.println(published ? "YES" : "NO");
  }

  delay(10000);
}

void reconnectMQTT() {
  if (mqttClient.connected()) {
    return;
  }

  Serial.print("Reconnecting to MQTT...");

  if (mqttClient.connect("hydro_01")) {
    Serial.println("connected!");
  } else {
    Serial.print("failed, state=");
    Serial.println(mqttClient.state());
  }
}