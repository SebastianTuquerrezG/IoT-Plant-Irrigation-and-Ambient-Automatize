#pragma once
#ifndef _ACTUATORS_LIB_H
#define _ACTUATORS_LIB_H

#include <Arduino.h>

// Estructura para almacenar estados de actuadores
struct ActuatorState {
  bool bomba;
  bool ventilador;
  bool deshumidificador;
  bool buzzer;
  int rgbR;
  int rgbG;
  int rgbB;
};

// Estructura para modos de control
struct ActuatorModes {
  bool bombaManual;
  bool ventiladorManual;
  bool deshumidificadorManual;
};

class ActuatorsLib {
public:
  ActuatorsLib();
  
  /**
   * @brief Inicializa todos los actuadores
   */
  void begin(int motor1A, int motor2A, int motor3A, int motor4A,
             int buzzerPin, int redPin, int greenPin, int bluePin);
  
  // Control de Bomba de Riego
  void setBomba(bool estado);
  bool getBomba();
  void setBombaManual(bool manual);
  bool isBombaManual();
  
  // Control de Ventilador
  void setVentilador(bool estado);
  bool getVentilador();
  void setVentiladorManual(bool manual);
  bool isVentiladorManual();
  
  // Control de Deshumidificador
  void setDeshumidificador(bool estado);
  bool getDeshumidificador();
  void setDeshumidificadorManual(bool manual);
  bool isDeshumidificadorManual();
  
  // Control de Buzzer
  void setBuzzer(bool estado);
  bool getBuzzer();
  
  // Control de LED RGB
  void setRGB(int r, int g, int b);
  void setRGBByZone(int zone); // 0=azul, 1=verde, 2=rojo
  
  // Obtener estados
  ActuatorState getState();
  ActuatorModes getModes();
  
  // Control por comando string
  bool executeCommand(String actuador, String comando);
  
private:
  int _motor1A, _motor2A, _motor3A, _motor4A;
  int _buzzerPin;
  int _redPin, _greenPin, _bluePin;
  bool _rgbCommonAnode;
  
  ActuatorState _state;
  ActuatorModes _modes;
  
  void applyRGB();
};

#endif

