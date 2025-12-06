#pragma once
#ifndef _SENSORS_LIB_H
#define _SENSORS_LIB_H

#include <Arduino.h>
#include <DHT.h>

// Estructura para almacenar todas las lecturas de sensores
struct SensorData {
  float temperatura;
  float humedadAmbiental;
  bool humedadSuelo;      // true = SECO, false = HUMEDO
  int luminosidad;
  bool movimiento;
  int zonaLuz;            // 0=oscuro, 1=medio, 2=brillante
};

class SensorsLib {
public:
  SensorsLib();
  
  /**
   * @brief Inicializa todos los sensores
   * @param dhtPin Pin del sensor DHT11
   * @param humedadPin Pin del sensor de humedad de suelo HW-080
   * @param pirPin Pin del sensor PIR
   * @param ldrPin Pin del sensor LDR
   */
  void begin(int dhtPin, int humedadPin, int pirPin, int ldrPin);
  
  /**
   * @brief Actualiza las lecturas de todos los sensores
   * Debe llamarse en el loop principal
   */
  void update();
  
  /**
   * @brief Obtiene los datos actuales de todos los sensores
   * @return SensorData estructura con todas las lecturas
   */
  SensorData getData();
  
  // Métodos individuales para cada sensor
  float getTemperatura();
  float getHumedadAmbiental();
  bool getHumedadSuelo();
  int getLuminosidad();
  bool getMovimiento();
  int getZonaLuz();
  
  // Configuración del filtro PIR
  void setPirConsecutiveRequired(int count);
  void setPirHoldTime(unsigned long ms);
  
private:
  DHT* _dht;
  int _dhtPin;
  int _humedadPin;
  int _pirPin;
  int _ldrPin;
  
  SensorData _data;
  
  // Timers
  unsigned long _lastDHTRead;
  unsigned long _lastHumedadRead;
  unsigned long _lastLDRRead;
  unsigned long _lastPIRRead;
  
  // Intervalos de lectura (ms)
  static const unsigned long INTERVALO_DHT = 2000;
  static const unsigned long INTERVALO_HUMEDAD = 200;
  static const unsigned long INTERVALO_LDR = 300;
  static const unsigned long INTERVALO_PIR = 50;
  
  // PIR filtro
  int _pirConsecutiveRequired;
  int _pirConsecutiveCount;
  unsigned long _lastMotionTime;
  unsigned long _pirHoldTime;
  bool _motionState;
  
  // Métodos privados de lectura
  void readDHT();
  void readHumedadSuelo();
  void readLDR();
  void readPIR();
};

#endif

