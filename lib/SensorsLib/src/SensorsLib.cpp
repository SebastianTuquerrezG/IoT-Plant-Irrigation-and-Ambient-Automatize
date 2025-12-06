#include "SensorsLib.h"

SensorsLib::SensorsLib() {
  _dht = nullptr;
  _pirConsecutiveRequired = 3;
  _pirConsecutiveCount = 0;
  _pirHoldTime = 3000;
  _motionState = false;
  _lastMotionTime = 0;
  
  _lastDHTRead = 0;
  _lastHumedadRead = 0;
  _lastLDRRead = 0;
  _lastPIRRead = 0;
  
  _data.temperatura = 0;
  _data.humedadAmbiental = 0;
  _data.humedadSuelo = false;
  _data.luminosidad = 0;
  _data.movimiento = false;
  _data.zonaLuz = 1;
}

void SensorsLib::begin(int dhtPin, int humedadPin, int pirPin, int ldrPin) {
  _dhtPin = dhtPin;
  _humedadPin = humedadPin;
  _pirPin = pirPin;
  _ldrPin = ldrPin;
  
  // Inicializar DHT
  _dht = new DHT(dhtPin, DHT11);
  _dht->begin();
  
  // Configurar pines
  pinMode(_humedadPin, INPUT);
  pinMode(_pirPin, INPUT);
  // LDR no necesita pinMode para analogRead
  
  Serial.println("[SensorsLib] Sensores inicializados");
}

void SensorsLib::update() {
  unsigned long ahora = millis();
  
  // Leer DHT11
  if (ahora - _lastDHTRead >= INTERVALO_DHT) {
    _lastDHTRead = ahora;
    readDHT();
  }
  
  // Leer humedad de suelo
  if (ahora - _lastHumedadRead >= INTERVALO_HUMEDAD) {
    _lastHumedadRead = ahora;
    readHumedadSuelo();
  }
  
  // Leer LDR
  if (ahora - _lastLDRRead >= INTERVALO_LDR) {
    _lastLDRRead = ahora;
    readLDR();
  }
  
  // Leer PIR
  if (ahora - _lastPIRRead >= INTERVALO_PIR) {
    _lastPIRRead = ahora;
    readPIR();
  }
}

void SensorsLib::readDHT() {
  float temp = _dht->readTemperature();
  float hum = _dht->readHumidity();
  
  if (!isnan(temp) && !isnan(hum)) {
    _data.temperatura = temp;
    _data.humedadAmbiental = hum;
  }
}

void SensorsLib::readHumedadSuelo() {
  int lectura = digitalRead(_humedadPin);
  _data.humedadSuelo = (lectura == HIGH); // HIGH = seco
}

void SensorsLib::readLDR() {
  _data.luminosidad = analogRead(_ldrPin);
  
  // Calcular zona de luz
  if (_data.luminosidad < 20) {
    _data.zonaLuz = 0; // Oscuro
  } else if (_data.luminosidad < 500) {
    _data.zonaLuz = 1; // Medio
  } else {
    _data.zonaLuz = 2; // Brillante
  }
}

void SensorsLib::readPIR() {
  unsigned long ahora = millis();
  int lectura = digitalRead(_pirPin);
  
  if (lectura == HIGH) {
    _pirConsecutiveCount++;
  } else {
    _pirConsecutiveCount = 0;
  }
  
  // Activar movimiento si hay suficientes lecturas consecutivas
  if (_pirConsecutiveCount >= _pirConsecutiveRequired) {
    _pirConsecutiveCount = 0;
    _lastMotionTime = ahora;
    _motionState = true;
  }
  
  // Desactivar después del tiempo de hold
  if (_motionState && (ahora - _lastMotionTime >= _pirHoldTime)) {
    _motionState = false;
  }
  
  _data.movimiento = _motionState;
}

SensorData SensorsLib::getData() {
  return _data;
}

float SensorsLib::getTemperatura() {
  return _data.temperatura;
}

float SensorsLib::getHumedadAmbiental() {
  return _data.humedadAmbiental;
}

bool SensorsLib::getHumedadSuelo() {
  return _data.humedadSuelo;
}

int SensorsLib::getLuminosidad() {
  return _data.luminosidad;
}

bool SensorsLib::getMovimiento() {
  return _data.movimiento;
}

int SensorsLib::getZonaLuz() {
  return _data.zonaLuz;
}

void SensorsLib::setPirConsecutiveRequired(int count) {
  _pirConsecutiveRequired = count;
}

void SensorsLib::setPirHoldTime(unsigned long ms) {
  _pirHoldTime = ms;
}

