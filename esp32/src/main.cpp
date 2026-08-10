#include <WiFi.h>
#include <WebSocketsClient.h>
#include <MQTTPubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// ---------- BME280 ----------
Adafruit_BME280 bme;

// ---------- WIFI ----------
const char* ssid = "Totalplay-2.4G-6130";
const char* wifi_password = "ztwZQ5ZndM27SNhb";

// ---------- MQTT ----------
const char* mqtt_host = "weather.blyndthor.com";
const int mqtt_port = 443;

const char* mqtt_user = "telegraf";
const char* mqtt_password = "IHs2N1YtGrxCqDtT";

const char* mqtt_topic = "weather/esp32_c3_station";

// ---------- CLIENTES ----------
WebSocketsClient ws;
MQTTPubSubClient mqtt;

unsigned long lastMsg = 0;
const unsigned long interval = 60000;

void conectarWiFi() {
  Serial.print("Conectando WiFi");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("WiFi conectado. IP: ");
  Serial.println(WiFi.localIP());
}

void conectarMQTT() {
  Serial.println("Conectando WebSocket...");
  ws.beginSSL(mqtt_host, mqtt_port, "/");
  ws.setExtraHeaders("Sec-WebSocket-Protocol: mqtt");

  mqtt.begin(ws);

  Serial.println("Conectando MQTT...");

  while (!mqtt.connect(
      "esp32_c3_station",
      mqtt_user,
      mqtt_password
  )) {
    Serial.println("MQTT fallo. Reintentando...");
    delay(2000);
  }

  Serial.println("MQTT conectado.");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Wire.begin(8, 9);

  if (!bme.begin(0x76, &Wire)) {
    Serial.println("BME280 no encontrado en 0x76.");

    if (!bme.begin(0x77, &Wire)) {
      Serial.println("BME280 tampoco encontrado en 0x77.");
      while (true) {
        delay(1000);
      }
    }
  }

  Serial.println("BME280 detectado.");

  conectarWiFi();
  conectarMQTT();
}

void loop() {
  ws.loop();
  mqtt.update();

  if (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  }

  if (!mqtt.isConnected()) {
    conectarMQTT();
  }

  unsigned long now = millis();

  if (now - lastMsg >= interval) {
    lastMsg = now;

    float temperatura = bme.readTemperature();
    float humedad = bme.readHumidity();
    float presion = bme.readPressure() / 100.0F;

    Serial.print("Temperatura: ");
    Serial.print(temperatura);
    Serial.println(" C");

    Serial.print("Humedad: ");
    Serial.print(humedad);
    Serial.println(" %");

    Serial.print("Presion: ");
    Serial.print(presion);
    Serial.println(" hPa");

    JsonDocument doc;

    doc["device"] = "esp32_c3_station";
    doc["temperature"] = temperatura;
    doc["humidity"] = humedad;
    doc["pressure"] = presion;

    char buffer[256];
    serializeJson(doc, buffer);

    mqtt.publish(mqtt_topic, buffer);

    Serial.print("Publicado en ");
    Serial.print(mqtt_topic);
    Serial.print(": ");
    Serial.println(buffer);
  }
}
