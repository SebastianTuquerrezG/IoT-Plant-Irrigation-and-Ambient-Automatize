#include "ActuatorsLib.h"

ActuatorsLib::ActuatorsLib() {
  _rgbCommonAnode = false;
  
  _state.bomba = false;
  _state.ventilador = false;
  _state.deshumidificador = false;
  _state.buzzer = false;
  _state.rgbR = 0;
  _state.rgbG = 0;
  _state.rgbB = 0;
  
  _modes.bombaManual = false;
  _modes.ventiladorManual = false;
  _modes.deshumidificadorManual = false;
}

void ActuatorsLib::begin(int motor1A, int motor2A, int motor3A, int motor4A,
                         int buzzerPin, int redPin, int greenPin, int bluePin) {
  _motor1A = motor1A;
  _motor2A = motor2A;
  _motor3A = motor3A;
  _motor4A = motor4A;
  _buzzerPin = buzzerPin;
  _redPin = redPin;
  _greenPin = greenPin;
  _bluePin = bluePin;
  
  // Configurar pines como salida
  pinMode(_motor1A, OUTPUT);
  pinMode(_motor2A, OUTPUT);
  pinMode(_motor3A, OUTPUT);
  pinMode(_motor4A, OUTPUT);
  pinMode(_buzzerPin, OUTPUT);
  pinMode(_redPin, OUTPUT);
  pinMode(_greenPin, OUTPUT);
  pinMode(_bluePin, OUTPUT);
  
  // Estado inicial: todo apagado
  digitalWrite(_motor1A, LOW);
  digitalWrite(_motor2A, LOW);
  digitalWrite(_motor3A, LOW);
  digitalWrite(_motor4A, LOW);
  digitalWrite(_buzzerPin, LOW);
  
  Serial.println("[ActuatorsLib] Actuadores inicializados");
}

// ========== BOMBA ==========
void ActuatorsLib::setBomba(bool estado) {
  _state.bomba = estado;
  if (estado) {
    digitalWrite(_motor1A, HIGH);
    digitalWrite(_motor2A, LOW);
  } else {
    digitalWrite(_motor1A, LOW);
    digitalWrite(_motor2A, LOW);
  }
}

bool ActuatorsLib::getBomba() {
  return _state.bomba;
}

void ActuatorsLib::setBombaManual(bool manual) {
  _modes.bombaManual = manual;
}

bool ActuatorsLib::isBombaManual() {
  return _modes.bombaManual;
}

// ========== VENTILADOR ==========
void ActuatorsLib::setVentilador(bool estado) {
  _state.ventilador = estado;
  digitalWrite(_motor3A, estado ? HIGH : LOW);
}

bool ActuatorsLib::getVentilador() {
  return _state.ventilador;
}

void ActuatorsLib::setVentiladorManual(bool manual) {
  _modes.ventiladorManual = manual;
}

bool ActuatorsLib::isVentiladorManual() {
  return _modes.ventiladorManual;
}

// ========== DESHUMIDIFICADOR ==========
void ActuatorsLib::setDeshumidificador(bool estado) {
  _state.deshumidificador = estado;
  digitalWrite(_motor4A, estado ? HIGH : LOW);
}

bool ActuatorsLib::getDeshumidificador() {
  return _state.deshumidificador;
}

void ActuatorsLib::setDeshumidificadorManual(bool manual) {
  _modes.deshumidificadorManual = manual;
}

bool ActuatorsLib::isDeshumidificadorManual() {
  return _modes.deshumidificadorManual;
}

// ========== BUZZER ==========
void ActuatorsLib::setBuzzer(bool estado) {
  _state.buzzer = estado;
  digitalWrite(_buzzerPin, estado ? HIGH : LOW);
}

bool ActuatorsLib::getBuzzer() {
  return _state.buzzer;
}

// ========== LED RGB ==========
void ActuatorsLib::setRGB(int r, int g, int b) {
  _state.rgbR = r;
  _state.rgbG = g;
  _state.rgbB = b;
  applyRGB();
}

void ActuatorsLib::setRGBByZone(int zone) {
  switch (zone) {
    case 0: setRGB(0, 0, 255); break;   // Oscuro -> Azul
    case 1: setRGB(0, 255, 0); break;   // Medio -> Verde
    case 2: setRGB(255, 0, 0); break;   // Brillante -> Rojo
    default: setRGB(0, 0, 0); break;
  }
}

void ActuatorsLib::applyRGB() {
  int r = _state.rgbR;
  int g = _state.rgbG;
  int b = _state.rgbB;
  
  if (_rgbCommonAnode) {
    r = 255 - r;
    g = 255 - g;
    b = 255 - b;
  }
  
  analogWrite(_redPin, r);
  analogWrite(_greenPin, g);
  analogWrite(_bluePin, b);
}

// ========== ESTADOS ==========
ActuatorState ActuatorsLib::getState() {
  return _state;
}

ActuatorModes ActuatorsLib::getModes() {
  return _modes;
}

// ========== COMANDO POR STRING ==========
bool ActuatorsLib::executeCommand(String actuador, String comando) {
  actuador.toLowerCase();
  comando.toUpperCase();
  
  if (actuador == "bomba") {
    if (comando == "ON") {
      setBombaManual(true);
      setBomba(true);
      return true;
    } else if (comando == "OFF") {
      setBombaManual(true);
      setBomba(false);
      return true;
    } else if (comando == "AUTO") {
      setBombaManual(false);
      return true;
    }
  }
  else if (actuador == "ventilador") {
    if (comando == "ON") {
      setVentiladorManual(true);
      setVentilador(true);
      return true;
    } else if (comando == "OFF") {
      setVentiladorManual(true);
      setVentilador(false);
      return true;
    } else if (comando == "AUTO") {
      setVentiladorManual(false);
      return true;
    }
  }
  else if (actuador == "deshumidificador") {
    if (comando == "ON") {
      setDeshumidificadorManual(true);
      setDeshumidificador(true);
      return true;
    } else if (comando == "OFF") {
      setDeshumidificadorManual(true);
      setDeshumidificador(false);
      return true;
    } else if (comando == "AUTO") {
      setDeshumidificadorManual(false);
      return true;
    }
  }
  
  return false;
}

