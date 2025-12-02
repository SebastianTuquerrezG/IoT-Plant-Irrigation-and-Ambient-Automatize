#include <Arduino.h>
#include <DHT.h>

#define motor1A 17
#define motor2A 25
#define motor3A 16
#define motor4A 27
#define sensorPin 23  // Sensor de humedad HW-080 (DO)
const int PIRPin = 18;

#define DHTPIN 19     // Pin del DHT11
#define DHTTYPE DHT11 // Tipo de sensor

DHT dht(DHTPIN, DHTTYPE);
unsigned long lastDHT = 0;
const unsigned long intervaloDHT = 2000;  // cada 2s

#define BUZZER_PIN 26
#define RED_PIN 5         
#define GREEN_PIN 13
#define BLUE_PIN 12 

const int LDR_PIN = 39;

bool RGB_COMMON_ANODE = false;

// Timers
unsigned long lastHumedadCheck = 0;
const unsigned long intervaloHumedad = 200; // ms

unsigned long lastLDRCheck = 0;
const unsigned long intervaloLDR = 300;   // <--- LDR PARARELO

// PIR timers
unsigned long lastPIRCheck = 0;
const unsigned long intervaloPIR = 50;
const int requiredConsecutive = 3;
unsigned long lastMotionTime = 0;
const unsigned long motionHoldMs = 3000;
int consecutiveHigh = 0;
bool motionState = false;

bool bombaEncendida = false;
bool motor3Encendido = false;
bool motor4Encendido = false;

// UMBRALES DHT11 para control de motores 3 y 4
const float TEMP_MAX = 19.0;      // Temperatura máxima para activar ventilación (motor3)
const float TEMP_MIN = 15.0;      // Temperatura mínima para desactivar ventilación
const float HUMEDAD_MAX = 70.0;   // Humedad máxima para activar deshumidificador (motor4)
const float HUMEDAD_MIN = 50.0;   // Humedad mínima para desactivar deshumidificador

void setRGB(int r, int g, int b) {
  if (RGB_COMMON_ANODE) {
    r = 255 - r; g = 255 - g; b = 255 - b;
  }
  analogWrite(RED_PIN, r);
  analogWrite(GREEN_PIN, g);
  analogWrite(BLUE_PIN, b);
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Sistema iniciado...");

  // Inicializar DHT
  dht.begin();
  delay(1000);

  pinMode(motor1A, OUTPUT);
  pinMode(motor2A, OUTPUT);
  pinMode(motor3A, OUTPUT);
  pinMode(motor4A, OUTPUT);
  pinMode(sensorPin, INPUT);
  pinMode(PIRPin, INPUT);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  digitalWrite(motor1A, LOW);
  digitalWrite(motor2A, LOW);
  digitalWrite(motor3A, LOW);
  digitalWrite(motor4A, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  delay(2000);
}

void loop() {
  unsigned long ahora = millis();

  // ----------- PIR -----------
  if (ahora - lastPIRCheck >= intervaloPIR) {
    lastPIRCheck = ahora;

    int lectura = digitalRead(PIRPin);
    if (lectura == HIGH) consecutiveHigh++;
    else consecutiveHigh = 0;

    if (consecutiveHigh >= requiredConsecutive) {
      consecutiveHigh = 0;
      lastMotionTime = ahora;

      if (!motionState) {
        motionState = true;
        Serial.println("Movimiento detectado → BUZZER ON");
      }
    }

    if (motionState && (ahora - lastMotionTime >= motionHoldMs)) {
      motionState = false;
      Serial.println("Hold terminado → BUZZER OFF");
    }

    digitalWrite(BUZZER_PIN, motionState ? HIGH : LOW);
  }

  // ----------- HUMEDAD (ESTE ERA TU PROBLEMA) -----------
  if (ahora - lastHumedadCheck >= intervaloHumedad) {
    lastHumedadCheck = ahora;

    int humedad = digitalRead(sensorPin);

    if (humedad == HIGH && !bombaEncendida) {
      Serial.println("Suelo SECO → Activando bomba");
      digitalWrite(motor1A, HIGH);
      digitalWrite(motor2A, LOW);
      bombaEncendida = true;
    } 
    else if (humedad == LOW && bombaEncendida) {
      Serial.println("Suelo HUMEDO → Bomba OFF");
      digitalWrite(motor1A, LOW);
      digitalWrite(motor2A, LOW);
      bombaEncendida = false;
    }
  }

  // ----------- LDR (PROCESO PARALELO REAL) -----------
  if (ahora - lastLDRCheck >= intervaloLDR) {
    lastLDRCheck = ahora;

    int ldr = analogRead(LDR_PIN);
    Serial.print("LDR"); Serial.println((int)ldr);

    // Solo mostramos cuando cambia de zona
    static int zona = -1;
    int nuevaZona;

    if (ldr < 20)          nuevaZona = 0;  // oscuro
    else if (ldr >= 20 && ldr < 500)     nuevaZona = 1;  // media
    else                    nuevaZona = 2;  // mucha luz

    if (zona != nuevaZona) {
      zona = nuevaZona;

      Serial.print("LDR: ");
      Serial.print(ldr);
      Serial.print(" → Zona: ");
      Serial.println(zona);

      if (zona == 0)      setRGB(0, 0, 255);
      else if (zona == 1) setRGB(0, 255, 0);
      else                setRGB(255, 0, 0);
    }
  }

  // ----------- DHT11 -----------
  if (ahora - lastDHT >= intervaloDHT) {
    lastDHT = ahora;

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    // Validar lecturas
    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("❌ Error leyendo DHT11 - Verifica conexión");
    } else {
      Serial.println("=================================");
      Serial.print("Temp: "); Serial.print(temperature);
      Serial.print("°C | Humedad: "); Serial.print(humidity);
      Serial.println("%");

      // ----------- CONTROL MOTOR 3 (Ventilación) por TEMPERATURA -----------
      if (temperature >= TEMP_MAX && !motor3Encendido) {
        Serial.println("⚠️ Temperatura ALTA → Activando ventilador (Motor 3)");
        digitalWrite(motor3A, HIGH);
        motor3Encendido = true;
      }
      else if (temperature <= TEMP_MIN && motor3Encendido) {
        Serial.println("✓ Temperatura normal → Desactivando ventilador (Motor 3)");
        digitalWrite(motor3A, LOW);
        motor3Encendido = false;
      }

      // ----------- CONTROL MOTOR 4 (Deshumidificador) por HUMEDAD -----------
      if (humidity >= HUMEDAD_MAX && !motor4Encendido) {
        Serial.println("⚠️ Humedad ALTA → Activando deshumidificador (Motor 4)");
        digitalWrite(motor4A, HIGH);
        motor4Encendido = true;
      }
      else if (humidity <= HUMEDAD_MIN && motor4Encendido) {
        Serial.println("✓ Humedad normal → Desactivando deshumidificador (Motor 4)");
        digitalWrite(motor4A, LOW);
        motor4Encendido = false;
      }
    }
  }
}