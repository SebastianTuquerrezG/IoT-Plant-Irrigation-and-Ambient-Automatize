#pragma once
#ifndef _MQTT_LIB_H
#define _MQTT_LIB_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "SensorsLib.h"
#include "ActuatorsLib.h"

class MqttLib {
public:
  MqttLib();
  
  /**
   * @brief Inicializa la conexión MQTT
   * @param server Dirección del broker MQTT
   * @param port Puerto del broker (generalmente 1883)
   * @param sensors Referencia a la librería de sensores
   * @param actuators Referencia a la librería de actuadores
   */
  void begin(const char* server, int port, SensorsLib* sensors, ActuatorsLib* actuators);
  
  /**
   * @brief Mantiene la conexión MQTT activa
   * Debe llamarse en el loop principal
   */
  void loop();
  
  /**
   * @brief Publica todos los datos de sensores y actuadores
   */
  void publishData();
  
  /**
   * @brief Verifica si está conectado al broker
   */
  bool isConnected();
  
  /**
   * @brief Establece el intervalo de publicación
   */
  void setPublishInterval(unsigned long ms);
  
  /**
   * @brief Obtiene el cliente MQTT para configuración adicional
   */
  PubSubClient* getClient();
  
private:
  WiFiClient _espClient;
  PubSubClient _client;
  SensorsLib* _sensors;
  ActuatorsLib* _actuators;
  
  const char* _server;
  int _port;
  
  unsigned long _lastPublish;
  unsigned long _publishInterval;
  
  // Topics de publicación
  static constexpr const char* TOPIC_TEMPERATURA = "invernadero/sensores/temperatura";
  static constexpr const char* TOPIC_HUMEDAD_AMB = "invernadero/sensores/humedad_ambiental";
  static constexpr const char* TOPIC_HUMEDAD_SUELO = "invernadero/sensores/humedad_suelo";
  static constexpr const char* TOPIC_LUMINOSIDAD = "invernadero/sensores/luminosidad";
  static constexpr const char* TOPIC_MOVIMIENTO = "invernadero/sensores/movimiento";
  static constexpr const char* TOPIC_ESTADO_BOMBA = "invernadero/actuadores/bomba/estado";
  static constexpr const char* TOPIC_ESTADO_VENTILADOR = "invernadero/actuadores/ventilador/estado";
  static constexpr const char* TOPIC_ESTADO_DESHUM = "invernadero/actuadores/deshumidificador/estado";
  static constexpr const char* TOPIC_ESTADO_BUZZER = "invernadero/actuadores/buzzer/estado";
  
  // Topics de suscripción (comandos)
  static constexpr const char* TOPIC_CMD_BOMBA = "invernadero/actuadores/bomba/comando";
  static constexpr const char* TOPIC_CMD_VENTILADOR = "invernadero/actuadores/ventilador/comando";
  static constexpr const char* TOPIC_CMD_DESHUM = "invernadero/actuadores/deshumidificador/comando";
  
  void reconnect();
  void subscribeToCommands();
  static void callbackWrapper(char* topic, byte* payload, unsigned int length);
  void handleCallback(char* topic, byte* payload, unsigned int length);
};

// Instancia global para el callback
extern MqttLib* _mqttInstance;

#endif

