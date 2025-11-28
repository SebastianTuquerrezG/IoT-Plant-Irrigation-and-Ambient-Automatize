#include <Arduino.h>
#include <SimpleDHT.h>

#define motor1A 13
#define motor2A 12
#define sensorPin 11   // Sensor de humedad HW-080 (DO)
const int PIRPin = 9;

int pinDHT11 = 8;
SimpleDHT11 dht11;
unsigned long lastDHT = 0;
const unsigned long intervaloDHT = 2000;  // cada 2s

#define BUZZER_PIN 10     
#define RED_PIN 3         
#define GREEN_PIN 5
#define BLUE_PIN 4 

const int LDR_PIN = A0;

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

void setRGB(int r, int g, int b) {
  if (RGB_COMMON_ANODE) {
    r = 255 - r; g = 255 - g; b = 255 - b;
  }
  analogWrite(RED_PIN, r);
  analogWrite(GREEN_PIN, g);
  analogWrite(BLUE_PIN, b);
}

void setup() {
  Serial.begin(9600);
  Serial.println("Sistema iniciado...");

  pinMode(motor1A, OUTPUT);
  pinMode(motor2A, OUTPUT);
  pinMode(sensorPin, INPUT);
  pinMode(PIRPin, INPUT);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  digitalWrite(motor1A, LOW);
  digitalWrite(motor2A, LOW);
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
    else if (ldr >= 20 && ldr < 100)     nuevaZona = 1;  // media
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

    byte temperature = 0;
    byte humidity = 0;
    byte data[40] = {0};

    if (!dht11.read(pinDHT11, &temperature, &humidity, data)) {
      Serial.println("=================================");
      Serial.print("Temp: "); Serial.println((int)temperature);
      Serial.print("Humedad: "); Serial.println((int)humidity);
    } else {
      Serial.println("Error leyendo DHT11");
    }
  }
}