#pragma once
#ifndef _METADATA_LIB_H
#define _METADATA_LIB_H

#include <Arduino.h>
#include <WiFi.h>
#include "SensorsLib.h"
#include "ActuatorsLib.h"

// Estructura de configuración del dispositivo
struct DeviceConfig {
  String id;
  String tipo;
  String fabricante;
  String descripcion;
  String mqttBroker;
  int mqttPort;
  float tempMax;
  float tempMin;
  float humedadMax;
  float humedadMin;
};

class MetadataLib {
public:
  MetadataLib();
  
  /**
   * @brief Configura los datos del dispositivo
   */
  void setConfig(DeviceConfig config);
  
  /**
   * @brief Establece las referencias a sensores y actuadores
   */
  void setReferences(SensorsLib* sensors, ActuatorsLib* actuators);
  
  /**
   * @brief Genera JSON con información del dispositivo (metadatos)
   */
  String getDeviceJSON();
  
  /**
   * @brief Genera JSON con estado actual de sensores y actuadores
   */
  String getStatusJSON();
  
  /**
   * @brief Genera respuesta JSON para comandos de actuadores
   */
  String getCommandResponseJSON(String actuador, String estado, String modo);
  
  /**
   * @brief Obtiene la configuración actual
   */
  DeviceConfig getConfig();
  
  // ========== MÉTODOS PARA MODIFICAR UMBRALES ==========
  
  /**
   * @brief Establece el umbral máximo de humedad ambiental
   */
  void setHumedadMax(float valor);
  
  /**
   * @brief Establece el umbral mínimo de humedad ambiental
   */
  void setHumedadMin(float valor);
  
  /**
   * @brief Establece el umbral máximo de temperatura
   */
  void setTempMax(float valor);
  
  /**
   * @brief Establece el umbral mínimo de temperatura
   */
  void setTempMin(float valor);
  
  /**
   * @brief Genera JSON con los umbrales actuales
   */
  String getUmbralesJSON();
  
private:
  DeviceConfig _config;
  SensorsLib* _sensors;
  ActuatorsLib* _actuators;
};

#endif
