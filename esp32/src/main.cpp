#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// --- CONFIGURACIÓN DE RED ---
const char* ssid = "Totalplay-2.4G-6130";
const char* password = "ztwZQ5ZndM27SNhb";

// --- CONFIGURACIÓN MQTT ---
const char* mqtt_server = "weather.blyndthor.com";
const int mqtt_port = 1883;
const char* mqtt_user = "tu_usuario_mqtt";
const char* mqtt_password = "tu_password_mqtt";

// --- TÓPICO MQTT ---
const char* mqtt_topic = "weather/sensors";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
const long interval = 5000;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a la red WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("¡WiFi conectado!");
  Serial.print("Dirección IP asignada: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    
    String clientId = "ESP32C3-Weather-";
    clientId += String((uint32_t)ESP.getEfuseMac(), HEX);

    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println(" ¡Conectado al broker MQTT!");
    } else {
      Serial.print(" Falló la conexión, rc=");
      Serial.print(client.state());
      Serial.println(" Reintentando en 5 segundos...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  setup_wifi();
  
  client.setServer(mqtt_server, mqtt_port);
  client.setBufferSize(512);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;

    float temperatura = 25.4 + (random(0, 20) / 10.0);
    float humedad     = 55.0 + (random(0, 50) / 10.0);
    float presion     = 1013.25 + (random(-10, 10) / 10.0);

    JsonDocument doc;
    doc["device"] = "esp32_c3_station";
    doc["temperature"] = temperatura;
    doc["humidity"] = humedad;
    doc["pressure"] = presion;

    char buffer[256];
    serializeJson(doc, buffer);

    Serial.print("Publicando en [");
    Serial.print(mqtt_topic);
    Serial.print("] -> ");
    Serial.println(buffer);
    
    client.publish(mqtt_topic, buffer);
  }
}
