#include "MqttLib.h"

// Instancia global para el callback estático
MqttLib* _mqttInstance = nullptr;

// Metadata constante
const char* METADATA_PAYLOAD = "{\"device\":\"ESP32 Invernadero\",\"location\":\"Casa\",\"sensors\":[\"temperatura\",\"humedad_ambiental\",\"humedad_suelo\",\"luminosidad\",\"movimiento\"],\"actuators\":[\"bomba\",\"ventilador\",\"deshumidificador\",\"buzzer\"]}";

MqttLib::MqttLib() : _client(_espClient) {
  _sensors = nullptr;
  _actuators = nullptr;
  _server = nullptr;
  _port = 1883;
  _lastPublish = 0;
  _publishInterval = 5000; // 5 segundos por defecto
  _mqttInstance = this;
  clientId = "ESP32_" + String(random(0xffff), HEX);
}

void MqttLib::begin(const char* server, int port, SensorsLib* sensors, ActuatorsLib* actuators) {
  _server = server;
  _port = port;
  _sensors = sensors;
  _actuators = actuators;
  
  _client.setServer(server, port);
  _client.setCallback(callbackWrapper);
  
  Serial.print("[MqttLib] Configurado para broker: ");
  Serial.print(server);
  Serial.print(":");
  Serial.println(port);
}

void MqttLib::publishMetadata(const char* payload) {
  _client.publish("invernadero/metadata", payload, true); // retained = true
}

void MqttLib::loop() {
  if (!_client.connected()) {
    reconnect();
  }
  _client.loop();
  
  // Publicar datos periódicamente
  unsigned long ahora = millis();
  if (ahora - _lastPublish >= _publishInterval) {
    _lastPublish = ahora;
    publishData();
  }
}

bool MqttLib::isConnected() {
  return _client.connected();
}

void MqttLib::setPublishInterval(unsigned long ms) {
  _publishInterval = ms;
}

PubSubClient* MqttLib::getClient() {
  return &_client;
}

void MqttLib::reconnect() {
  while (!_client.connected()) {
    Serial.print("[MqttLib] Conectando...");
    if (_client.connect(clientId.c_str())) {
      Serial.println(" Conectado!");
      subscribeToCommands();
      publishMetadata(METADATA_PAYLOAD);
    } else {
      Serial.print(" Error: ");
      Serial.print(_client.state());
      Serial.println(" Reintentando en 5s...");
      delay(5000);
    }
  }
}

void MqttLib::subscribeToCommands() {
  _client.subscribe(TOPIC_CMD_BOMBA);
  _client.subscribe(TOPIC_CMD_VENTILADOR);
  _client.subscribe(TOPIC_CMD_DESHUM);
  Serial.println("[MqttLib] Suscrito a topics de comandos");
}

void MqttLib::callbackWrapper(char* topic, byte* payload, unsigned int length) {
  if (_mqttInstance != nullptr) {
    _mqttInstance->handleCallback(topic, payload, length);
  }
}

void MqttLib::handleCallback(char* topic, byte* payload, unsigned int length) {
  String mensaje = "";
  for (unsigned int i = 0; i < length; i++) {
    mensaje += (char)payload[i];
  }
  
  Serial.print("[MqttLib] Mensaje [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(mensaje);
  
  String topicStr = String(topic);
  mensaje.toUpperCase();
  
  if (topicStr == TOPIC_CMD_BOMBA) {
    _actuators->executeCommand("bomba", mensaje);
  }
  else if (topicStr == TOPIC_CMD_VENTILADOR) {
    _actuators->executeCommand("ventilador", mensaje);
  }
  else if (topicStr == TOPIC_CMD_DESHUM) {
    _actuators->executeCommand("deshumidificador", mensaje);
  }
}

void MqttLib::publishData() {
  if (_sensors == nullptr || _actuators == nullptr) return;
  
  char msg[50];
  SensorData sensorData = _sensors->getData();
  ActuatorState actuatorState = _actuators->getState();
  
  // Publicar sensores
  snprintf(msg, 50, "%.1f", sensorData.temperatura);
  _client.publish(TOPIC_TEMPERATURA, msg);
  
  snprintf(msg, 50, "%.1f", sensorData.humedadAmbiental);
  _client.publish(TOPIC_HUMEDAD_AMB, msg);
  
  _client.publish(TOPIC_HUMEDAD_SUELO, sensorData.humedadSuelo ? "SECO" : "HUMEDO");
  
  snprintf(msg, 50, "%d", sensorData.luminosidad);
  _client.publish(TOPIC_LUMINOSIDAD, msg);
  
  _client.publish(TOPIC_MOVIMIENTO, sensorData.movimiento ? "DETECTADO" : "SIN_MOVIMIENTO");
  
  // Publicar estados de actuadores
  _client.publish(TOPIC_ESTADO_BOMBA, actuatorState.bomba ? "ON" : "OFF");
  _client.publish(TOPIC_ESTADO_VENTILADOR, actuatorState.ventilador ? "ON" : "OFF");
  _client.publish(TOPIC_ESTADO_DESHUM, actuatorState.deshumidificador ? "ON" : "OFF");
  _client.publish(TOPIC_ESTADO_BUZZER, actuatorState.buzzer ? "ON" : "OFF");
  
  Serial.println("[MqttLib] Datos publicados");
  String payload = "{";
  payload += "\"temp\":" + String(sensorData.temperatura,1) + ",";
  payload += "\"hum\":" + String(sensorData.humedadAmbiental,1) + ",";
  payload += "\"soil\":\"" + String(sensorData.humedadSuelo ? "SECO" : "HUMEDO") + "\",";
  payload += "\"ldr\":" + String(sensorData.luminosidad) + ",";
  payload += "\"motion\":\"" + String(sensorData.movimiento ? "DETECTADO":"SIN_MOVIMIENTO") + "\"";
  payload += "}";
  _client.publish("invernadero/sensors/json", payload.c_str());
}