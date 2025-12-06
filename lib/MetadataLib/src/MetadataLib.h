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
   * @return String JSON con metadatos
   */
  String getDeviceJSON();
  
  /**
   * @brief Genera JSON con estado actual de sensores y actuadores
   * @return String JSON con estado
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
  
private:
  DeviceConfig _config;
  SensorsLib* _sensors;
  ActuatorsLib* _actuators;
};

#endif

