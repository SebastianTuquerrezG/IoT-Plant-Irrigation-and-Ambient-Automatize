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
   * @param sensors Referencia a la librería de sensores
   * @param actuators Referencia a la librería de actuadores
   * @param metadata Referencia a la librería de metadatos
   */
  void begin(SensorsLib* sensors, ActuatorsLib* actuators, MetadataLib* metadata);
  
  /**
   * @brief Procesa las peticiones del servidor
   * Debe llamarse en el loop principal
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
  
  // Handlers
  void handleRoot();
  void handleDeviceInfo();
  void handleStatus();
  void handleBombaOn();
  void handleBombaOff();
  void handleBombaAuto();
  void handleVentiladorOn();
  void handleVentiladorOff();
  void handleVentiladorAuto();
  void handleDeshumOn();
  void handleDeshumOff();
  void handleDeshumAuto();
  void handleNotFound();
};

#endif

