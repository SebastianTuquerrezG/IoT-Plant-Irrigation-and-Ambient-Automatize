#include "MetadataLib.h"

MetadataLib::MetadataLib() {
  _sensors = nullptr;
  _actuators = nullptr;
  
  // Valores por defecto
  _config.id = "ESP32-Invernadero";
  _config.tipo = "ESP32-DOIT-DevKit-V1";
  _config.fabricante = "Espressif";
  _config.descripcion = "Sistema de riego automatico y control ambiental";
  _config.mqttBroker = "test.mosquitto.org";
  _config.mqttPort = 1883;
  _config.tempMax = 19.0;
  _config.tempMin = 15.0;
  _config.humedadMax = 70.0;
  _config.humedadMin = 50.0;
  _config.luzBaja = 20;
  _config.luzAlta = 500;
}

void MetadataLib::setConfig(DeviceConfig config) {
  _config = config;
}

void MetadataLib::setReferences(SensorsLib* sensors, ActuatorsLib* actuators) {
  _sensors = sensors;
  _actuators = actuators;
}

DeviceConfig MetadataLib::getConfig() {
  return _config;
}

// ========== MÉTODOS PARA MODIFICAR UMBRALES ==========
void MetadataLib::setHumedadMax(float valor) {
  _config.humedadMax = valor;
  Serial.print("[MetadataLib] Umbral humedad max: ");
  Serial.println(valor);
}

void MetadataLib::setHumedadMin(float valor) {
  _config.humedadMin = valor;
  Serial.print("[MetadataLib] Umbral humedad min: ");
  Serial.println(valor);
}

void MetadataLib::setTempMax(float valor) {
  _config.tempMax = valor;
  Serial.print("[MetadataLib] Umbral temp max: ");
  Serial.println(valor);
}

void MetadataLib::setTempMin(float valor) {
  _config.tempMin = valor;
  Serial.print("[MetadataLib] Umbral temp min: ");
  Serial.println(valor);
}

void MetadataLib::setLuzBaja(int valor) {
  _config.luzBaja = valor;
  Serial.print("[MetadataLib] Umbral luz baja: ");
  Serial.println(valor);
}

void MetadataLib::setLuzAlta(int valor) {
  _config.luzAlta = valor;
  Serial.print("[MetadataLib] Umbral luz alta: ");
  Serial.println(valor);
}

String MetadataLib::getUmbralesJSON() {
  String json = "{";
  json += "\"humedad_max\":" + String(_config.humedadMax, 1) + ",";
  json += "\"humedad_min\":" + String(_config.humedadMin, 1) + ",";
  json += "\"temp_max\":" + String(_config.tempMax, 1) + ",";
  json += "\"temp_min\":" + String(_config.tempMin, 1) + ",";
  json += "\"luz_baja\":" + String(_config.luzBaja) + ",";
  json += "\"luz_alta\":" + String(_config.luzAlta);
  json += "}";
  return json;
}

String MetadataLib::getDeviceJSON() {
  String json = "{";
  json += "\"id\":\"" + _config.id + "\",";
  json += "\"tipo\":\"" + _config.tipo + "\",";
  json += "\"fabricante\":\"" + _config.fabricante + "\",";
  json += "\"descripcion\":\"" + _config.descripcion + "\",";
  json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
  
  // Sensores
  json += "\"sensores\":[";
  json += "{\"id\":1,\"nombre\":\"DHT11\",\"tipo\":\"temperatura_humedad\",\"gpio\":19,\"propiedades\":[\"temperatura\",\"humedad_ambiental\"]},";
  json += "{\"id\":2,\"nombre\":\"HW-080\",\"tipo\":\"humedad_suelo\",\"gpio\":23},";
  json += "{\"id\":3,\"nombre\":\"PIR\",\"tipo\":\"movimiento\",\"gpio\":18},";
  json += "{\"id\":4,\"nombre\":\"LDR\",\"tipo\":\"luminosidad\",\"gpio\":39}";
  json += "],";
  
  // Actuadores
  json += "\"actuadores\":[";
  json += "{\"id\":5,\"nombre\":\"BombaRiego\",\"tipo\":\"motor\",\"gpio\":\"17,25\",\"descripcion\":\"Bomba de agua para riego\"},";
  json += "{\"id\":6,\"nombre\":\"Deshumidificador\",\"tipo\":\"motor\",\"gpio\":27,\"descripcion\":\"Motor para control de humedad\"},";
  json += "{\"id\":7,\"nombre\":\"Buzzer\",\"tipo\":\"alarma\",\"gpio\":26,\"descripcion\":\"Alarma sonora de movimiento\"},";
  json += "{\"id\":8,\"nombre\":\"LED_RGB\",\"tipo\":\"indicador\",\"gpio\":\"5,13,12\",\"descripcion\":\"Indicador visual de luminosidad\"}";
  json += "],";
  
  // Umbrales
  json += "\"umbrales\":{";
  json += "\"humedad_max\":" + String(_config.humedadMax, 1) + ",";
  json += "\"humedad_min\":" + String(_config.humedadMin, 1) + ",";
  json += "\"temperatura_max\":" + String(_config.tempMax, 1) + ",";
  json += "\"temperatura_min\":" + String(_config.tempMin, 1) + ",";
  json += "\"luz_baja\":" + String(_config.luzBaja) + ",";
  json += "\"luz_alta\":" + String(_config.luzAlta);
  json += "},";
  
  // MQTT
  json += "\"mqtt\":{";
  json += "\"broker\":\"" + _config.mqttBroker + "\",";
  json += "\"puerto\":" + String(_config.mqttPort);
  json += "}";
  
  json += "}";
  return json;
}

String MetadataLib::getStatusJSON() {
  String json = "{";
  
  if (_sensors != nullptr) {
    SensorData data = _sensors->getData();
    
    json += "\"sensores\":{";
    json += "\"temperatura\":" + String(data.temperatura, 1) + ",";
    json += "\"humedad_ambiental\":" + String(data.humedadAmbiental, 1) + ",";
    json += "\"humedad_suelo\":\"" + String(data.humedadSuelo ? "SECO" : "HUMEDO") + "\",";
    json += "\"luminosidad\":" + String(data.luminosidad) + ",";
    json += "\"movimiento\":\"" + String(data.movimiento ? "DETECTADO" : "SIN_MOVIMIENTO") + "\"";
    json += "},";
  }
  
  if (_actuators != nullptr) {
    ActuatorState state = _actuators->getState();
    ActuatorModes modes = _actuators->getModes();
    
    json += "\"actuadores\":{";
    json += "\"bomba\":{\"estado\":\"" + String(state.bomba ? "ON" : "OFF") + "\",\"modo\":\"" + String(modes.bombaManual ? "MANUAL" : "AUTO") + "\"},";
    json += "\"deshumidificador\":{\"estado\":\"" + String(state.deshumidificador ? "ON" : "OFF") + "\",\"modo\":\"" + String(modes.deshumidificadorManual ? "MANUAL" : "AUTO") + "\"},";
    json += "\"buzzer\":{\"estado\":\"" + String(state.buzzer ? "ON" : "OFF") + "\"}";
    json += "},";
  }
  
  // Incluir umbrales actuales en el status
  json += "\"umbrales\":{";
  json += "\"humedad_max\":" + String(_config.humedadMax, 1) + ",";
  json += "\"humedad_min\":" + String(_config.humedadMin, 1) + ",";
  json += "\"temp_max\":" + String(_config.tempMax, 1) + ",";
  json += "\"temp_min\":" + String(_config.tempMin, 1) + ",";
  json += "\"luz_baja\":" + String(_config.luzBaja) + ",";
  json += "\"luz_alta\":" + String(_config.luzAlta);
  json += "}";
  
  json += "}";
  return json;
}

String MetadataLib::getCommandResponseJSON(String actuador, String estado, String modo) {
  String json = "{";
  json += "\"success\":true,";
  json += "\"actuador\":\"" + actuador + "\",";
  json += "\"estado\":\"" + estado + "\",";
  json += "\"modo\":\"" + modo + "\"";
  json += "}";
  return json;
}
