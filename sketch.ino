#include <WiFi.h>
#include <PubSubClient.h>
#include "DHTesp.h"
#include "secrets.h"

// ===== CONFIGURACIÓN =====
const int DHT_PIN = 14;
const int LED_PIN = 2;
const unsigned long LECTURA_MS = 2000;
const unsigned long PUBLICAR_MS = 5000;
const float UMBRAL_ALTO = 30.0;
const float UMBRAL_BAJO = 29.0;

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASS = "";
const int WIFI_CANAL = 6;

const char* MQTT_HOST = "mqtt.thingsboard.cloud";
const int MQTT_PORT = 1883;
const char* MQTT_CLIENT_ID = "esp32-dht-wokwi";
const char* MQTT_TOPIC = "v1/devices/me/telemetry";
const unsigned long REINTENTO_MQTT_MS = 5000;

DHTesp dht;
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

float temperatura = 0;
float humedad = 0;
bool hayDatos = false;
bool alarmaActiva = false;
unsigned long ultimaLectura = 0;
unsigned long ultimoEnvio = 0;
unsigned long ultimoIntentoMqtt = 0;

// ===== FUNCIONES: SENSOR Y ALARMA =====
void configurarSensor() {
  dht.setup(DHT_PIN, DHTesp::DHT22);
}

void configurarLed() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

bool lecturaLista() {
  return millis() - ultimaLectura >= LECTURA_MS;
}

bool publicacionLista() {
  return millis() - ultimoEnvio >= PUBLICAR_MS;
}

bool leerSensor(TempAndHumidity& datos) {
  datos = dht.getTempAndHumidity();
  if (dht.getStatus() != DHTesp::ERROR_NONE) {
    Serial.printf("Error de lectura: %s\n", dht.getStatusString());
    return false;
  }
  return true;
}

// Devuelve true si el estado de la alarma cambió
bool actualizarAlarma(float t) {
  bool anterior = alarmaActiva;
  if (!alarmaActiva && t > UMBRAL_ALTO) {
    alarmaActiva = true;
  } else if (alarmaActiva && t < UMBRAL_BAJO) {
    alarmaActiva = false;
  }
  digitalWrite(LED_PIN, alarmaActiva ? HIGH : LOW);
  return alarmaActiva != anterior;
}

void reportarSerial() {
  Serial.printf("Temperatura: %.1f C | Humedad: %.1f %% | Alarma: %s\n",
                temperatura, humedad, alarmaActiva ? "ON" : "OFF");
}

// ===== FUNCIONES: RED =====
void conectarWifi() {
  Serial.print("WiFi: conectando");
  WiFi.begin(WIFI_SSID, WIFI_PASS, WIFI_CANAL);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println(" OK");
}

void intentarMqtt() {
  ultimoIntentoMqtt = millis();
  Serial.print("MQTT: conectando... ");
  if (mqtt.connect(MQTT_CLIENT_ID, MQTT_TOKEN, NULL)) {
    Serial.println("OK");
  } else {
    Serial.printf("fallo rc=%d\n", mqtt.state());
  }
}

void asegurarMqtt() {
  if (mqtt.connected()) return;
  if (millis() - ultimoIntentoMqtt < REINTENTO_MQTT_MS) return;
  intentarMqtt();
}

void publicarTelemetria() {
  ultimoEnvio = millis();
  if (!hayDatos || !mqtt.connected()) return;
  char payload[80];
  snprintf(payload, sizeof(payload),
           "{\"temperatura\":%.1f,\"humedad\":%.1f,\"alarma\":%d}",
           temperatura, humedad, alarmaActiva ? 1 : 0);
  bool ok = mqtt.publish(MQTT_TOPIC, payload);
  Serial.printf("MQTT: %s -> %s\n", payload, ok ? "enviado" : "fallo");
}

// ===== LÓGICA PRINCIPAL =====
void procesarLectura() {
  ultimaLectura = millis();
  TempAndHumidity datos;
  if (!leerSensor(datos)) return;
  temperatura = datos.temperature;
  humedad = datos.humidity;
  hayDatos = true;
  bool cambio = actualizarAlarma(temperatura);
  reportarSerial();
  if (cambio) publicarTelemetria();
}

// ===== INICIALIZACIÓN =====
void setup() {
  Serial.begin(115200);
  configurarSensor();
  configurarLed();
  conectarWifi();
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  intentarMqtt();
}

// ===== BUCLE PRINCIPAL =====
void loop() {
  asegurarMqtt();
  mqtt.loop();
  if (lecturaLista()) procesarLectura();
  if (publicacionLista()) publicarTelemetria();
}