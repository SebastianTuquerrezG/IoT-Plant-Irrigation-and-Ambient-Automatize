/**
 * Sistema de Riego Automático y Control Ambiental IoT
 * 
 * Este proyecto implementa un sistema IoT completo con:
 * - 4 sensores (DHT11, HW-080, PIR, LDR)
 * - 3 actuadores (Bomba, Deshumidificador, Buzzer, LED RGB)
 * - Servidor web con dashboard y API REST
 * - Comunicación MQTT para telemetría y control remoto
 * 
 * El sensor de temperatura se usa solo para monitoreo.
 */

#include <Arduino.h>
#include <WiFiManager.h>

// Librerías propias del proyecto
#include <SensorsLib.h>
#include <ActuatorsLib.h>
#include <MetadataLib.h>
#include <WebServerLib.h>
#include <MqttLib.h>

// ==================== CONFIGURACIÓN DE PINES ====================
// Sensores
#define DHT_PIN         19
#define HUMEDAD_PIN     23    // HW-080
#define PIR_PIN         18
#define LDR_PIN         39

// Actuadores
#define MOTOR1A         17    // Bomba
#define MOTOR2A         25    // Bomba
//#define MOTOR3A         16    // Ventilador
#define MOTOR4A         27    // Deshumidificador
#define BUZZER_PIN      26
#define RED_PIN         12
#define GREEN_PIN       13
#define BLUE_PIN        5

// ==================== CONFIGURACIÓN DEL SISTEMA ====================
const char* MQTT_BROKER = "test.mosquitto.org";
const int MQTT_PORT = 1883;

// ==================== INSTANCIAS DE LIBRERÍAS ====================
SensorsLib sensors;
ActuatorsLib actuators;
MetadataLib metadata;
WebServerLib webServer;
MqttLib mqtt;

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n========================================");
  Serial.println("  Sistema de Riego Automatico IoT");
  Serial.println("  Version Modular con Librerias");
  Serial.println("========================================\n");

  // Inicializar sensores
  Serial.println("[Setup] Inicializando sensores...");
  sensors.begin(DHT_PIN, HUMEDAD_PIN, PIR_PIN, LDR_PIN);
  sensors.setPirConsecutiveRequired(3);
  sensors.setPirHoldTime(3000);

  // Inicializar actuadores
  Serial.println("[Setup] Inicializando actuadores...");
  actuators.begin(MOTOR1A, MOTOR2A,/*MOTOR3A,*/ MOTOR4A,
                  BUZZER_PIN, RED_PIN, GREEN_PIN, BLUE_PIN);

  // Configurar metadatos con umbrales iniciales
  Serial.println("[Setup] Configurando metadatos...");
  DeviceConfig config;
  config.id = "ESP32-Invernadero";
  config.tipo = "ESP32-DOIT-DevKit-V1";
  config.fabricante = "Espressif";
  config.descripcion = "Sistema de riego automatico y control ambiental";
  config.mqttBroker = MQTT_BROKER;
  config.mqttPort = MQTT_PORT;
  config.tempMax = 19.0;
  config.tempMin = 15.0;
  config.humedadMax = 70.0;
  config.humedadMin = 50.0;
  metadata.setConfig(config);
  metadata.setReferences(&sensors, &actuators);

  // Conectar WiFi
  Serial.println("[Setup] Iniciando WiFiManager...");
  WiFiManager wifiManager;
  wifiManager.autoConnect("ESP32-Invernadero");
  Serial.print("[Setup] WiFi conectado! IP: ");
  Serial.println(WiFi.localIP());

  // Inicializar servidor web
  Serial.println("[Setup] Iniciando servidor web...");
  webServer.begin(&sensors, &actuators, &metadata);

  // Inicializar MQTT
  Serial.println("[Setup] Iniciando cliente MQTT...");
  mqtt.begin(MQTT_BROKER, MQTT_PORT, &sensors, &actuators);
  mqtt.setPublishInterval(5000);

  Serial.println("\n[Setup] Sistema listo!");
  Serial.println("========================================\n");
}

// ==================== LOOP ====================
void loop() {
  // 1. Actualizar lecturas de sensores
  sensors.update();
  SensorData data = sensors.getData();

  // Obtener umbrales actuales (pueden cambiar desde la web)
  DeviceConfig config = metadata.getConfig();

  // 2. Lógica de automatización (solo si no está en modo manual)
  
  // Control de Bomba por humedad de suelo
  if (!actuators.isBombaManual()) {
    actuators.setBomba(data.humedadSuelo); // Seco (true) = Bomba ON
  }

  // El sensor de temperatura funciona para monitoreo

  // Control de Deshumidificador por humedad ambiental
  if (!actuators.isDeshumidificadorManual()) {
    if (data.humedadAmbiental >= config.humedadMax) {
      actuators.setDeshumidificador(true);
    } else if (data.humedadAmbiental <= config.humedadMin) {
      actuators.setDeshumidificador(false);
    }
  }

  // Control de Buzzer por movimiento (siempre automático)
  actuators.setBuzzer(data.movimiento);

  // Control de LED RGB por luminosidad (siempre automático)
  actuators.setRGBByZone(data.zonaLuz);

  // 3. Servicios de comunicación
  webServer.handleClient();  // Atender peticiones HTTP
  mqtt.loop();               // Mantener conexión MQTT y publicar datos
}
