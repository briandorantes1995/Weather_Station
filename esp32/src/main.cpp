#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// --- PANTALLA OLED ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- SENSOR BME280 ---
Adafruit_BME280 bme; // Usa dirección I2C por defecto (0x76 o 0x77)

// --- RED Y WEBSOCKET ---
const char* ssid = "Totalplay-2.4G-6130";
const char* password = "ztwZQ5ZndM27SNhb";
const char* mqtt_server = "weather.blyndthor.com";
const char* websocket_path = "/";

WebSocketsClient webSocket;
unsigned long lastMsg = 0;
const long interval = 5000;

void actualizarPantalla(String estado, float t, float h) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0, 0);
  display.print("Estado: ");
  display.println(estado);

  display.drawFastHLine(0, 12, 128, SSD1306_WHITE);

  display.setCursor(0, 20);
  display.setTextSize(2);
  display.print(t, 1);
  display.print(" C");

  display.setCursor(0, 42);
  display.print(h, 1);
  display.print(" % Hum");

  display.display();
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("[WS] Desconectado.");
      break;
    case WStype_CONNECTED:
      Serial.println("[WS] ¡Conectado a Caddy!");
      break;
    default:
      break;
  }
}

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}

void setup() {
  Serial.begin(115200);

  // Iniciar I2C en los pines por defecto del ESP32-C3 (GPIO 8 y 9)
  Wire.begin(8, 9);

  // Iniciar OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Fallo OLED"));
  }

  // Iniciar BME280 (Prueba con 0x76 si no lo detecta)
  if (!bme.begin(0x76, &Wire)) {
    Serial.println(F("No se encontró el sensor BME280, revisa el cableado!"));
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Conectando WiFi...");
  display.display();

  setup_wifi();

  webSocket.beginSSL(websocket_server, 443, websocket_path);
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
}

void loop() {
  webSocket.loop();

  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;

    if (WiFi.status() == WL_CONNECTED) {
      // --- LECTURA REAL DEL SENSOR ---
      float temperatura = bme.readTemperature();
      float humedad     = bme.readHumidity();
      float presion     = bme.readPressure() / 100.0F; // Convertir a hPa

      JsonDocument doc;
      doc["device"] = "esp32_c3_station";
      doc["temperature"] = temperatura;
      doc["humidity"] = humedad;
      doc["pressure"] = presion;

      char buffer[256];
      serializeJson(doc, buffer);

      webSocket.sendTXT(buffer);
      
      actualizarPantalla("Enviado", temperatura, humedad);
    } else {
      actualizarPantalla("Sin WiFi", 0.0, 0.0);
    }
  }
}
