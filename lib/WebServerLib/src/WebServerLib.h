#pragma once
#ifndef _WEBSERVER_LIB_H
#define _WEBSERVER_LIB_H

#include <Arduino.h>
#include <WebServer.h>
#include "SensorsLib.h"
#include "ActuatorsLib.h"
#include "MetadataLib.h"

class WebServerLib {
public:
  WebServerLib();
  
  /**
   * @brief Inicializa el servidor web
   */
  void begin(SensorsLib* sensors, ActuatorsLib* actuators, MetadataLib* metadata);
  
  /**
   * @brief Procesa las peticiones del servidor
   */
  void handleClient();
  
  /**
   * @brief Obtiene el servidor para configuraciones adicionales
   */
  WebServer* getServer();
  
private:
  WebServer _server;
  SensorsLib* _sensors;
  ActuatorsLib* _actuators;
  MetadataLib* _metadata;
  
  void setupRoutes();
  String getWebPage();
  
  // Handlers principales
  void handleRoot();
  void handleDeviceInfo();
  void handleStatus();
  void handleNotFound();
  
  // Handlers de Bomba
  void handleBombaOn();
  void handleBombaOff();
  void handleBombaAuto();
  
  // Handlers de Deshumidificador
  void handleDeshumOn();
  void handleDeshumOff();
  void handleDeshumAuto();
  
  // Handlers de Umbrales
  void handleSetUmbral();
  void handleGetUmbrales();
};

#endif
