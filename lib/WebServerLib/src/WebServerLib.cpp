#include "WebServerLib.h"

// Puntero estático para los callbacks
static WebServerLib* _instance = nullptr;

WebServerLib::WebServerLib() : _server(80) {
  _sensors = nullptr;
  _actuators = nullptr;
  _metadata = nullptr;
  _instance = this;
}

void WebServerLib::begin(SensorsLib* sensors, ActuatorsLib* actuators, MetadataLib* metadata) {
  _sensors = sensors;
  _actuators = actuators;
  _metadata = metadata;
  
  setupRoutes();
  _server.begin();
  
  Serial.println("[WebServerLib] Servidor web iniciado en puerto 80");
}

void WebServerLib::handleClient() {
  _server.handleClient();
}

WebServer* WebServerLib::getServer() {
  return &_server;
}

void WebServerLib::setupRoutes() {
  // Página principal
  _server.on("/", HTTP_GET, [this]() { this->handleRoot(); });
  
  // API REST - Metadatos y Estado
  _server.on("/api/device", HTTP_GET, [this]() { this->handleDeviceInfo(); });
  _server.on("/api/status", HTTP_GET, [this]() { this->handleStatus(); });
  
  // API REST - Bomba
  _server.on("/api/bomba/on", HTTP_GET, [this]() { this->handleBombaOn(); });
  _server.on("/api/bomba/off", HTTP_GET, [this]() { this->handleBombaOff(); });
  _server.on("/api/bomba/auto", HTTP_GET, [this]() { this->handleBombaAuto(); });
  
  // API REST - Ventilador
  _server.on("/api/ventilador/on", HTTP_GET, [this]() { this->handleVentiladorOn(); });
  _server.on("/api/ventilador/off", HTTP_GET, [this]() { this->handleVentiladorOff(); });
  _server.on("/api/ventilador/auto", HTTP_GET, [this]() { this->handleVentiladorAuto(); });
  
  // API REST - Deshumidificador
  _server.on("/api/deshumidificador/on", HTTP_GET, [this]() { this->handleDeshumOn(); });
  _server.on("/api/deshumidificador/off", HTTP_GET, [this]() { this->handleDeshumOff(); });
  _server.on("/api/deshumidificador/auto", HTTP_GET, [this]() { this->handleDeshumAuto(); });
  
  // 404
  _server.onNotFound([this]() { this->handleNotFound(); });
}

void WebServerLib::handleRoot() {
  _server.send(200, "text/html", getWebPage());
}

void WebServerLib::handleDeviceInfo() {
  _server.send(200, "application/json", _metadata->getDeviceJSON());
}

void WebServerLib::handleStatus() {
  _server.send(200, "application/json", _metadata->getStatusJSON());
}

void WebServerLib::handleBombaOn() {
  _actuators->executeCommand("bomba", "ON");
  _server.send(200, "application/json", _metadata->getCommandResponseJSON("bomba", "ON", "MANUAL"));
}

void WebServerLib::handleBombaOff() {
  _actuators->executeCommand("bomba", "OFF");
  _server.send(200, "application/json", _metadata->getCommandResponseJSON("bomba", "OFF", "MANUAL"));
}

void WebServerLib::handleBombaAuto() {
  _actuators->executeCommand("bomba", "AUTO");
  _server.send(200, "application/json", _metadata->getCommandResponseJSON("bomba", "-", "AUTO"));
}

void WebServerLib::handleVentiladorOn() {
  _actuators->executeCommand("ventilador", "ON");
  _server.send(200, "application/json", _metadata->getCommandResponseJSON("ventilador", "ON", "MANUAL"));
}

void WebServerLib::handleVentiladorOff() {
  _actuators->executeCommand("ventilador", "OFF");
  _server.send(200, "application/json", _metadata->getCommandResponseJSON("ventilador", "OFF", "MANUAL"));
}

void WebServerLib::handleVentiladorAuto() {
  _actuators->executeCommand("ventilador", "AUTO");
  _server.send(200, "application/json", _metadata->getCommandResponseJSON("ventilador", "-", "AUTO"));
}

void WebServerLib::handleDeshumOn() {
  _actuators->executeCommand("deshumidificador", "ON");
  _server.send(200, "application/json", _metadata->getCommandResponseJSON("deshumidificador", "ON", "MANUAL"));
}

void WebServerLib::handleDeshumOff() {
  _actuators->executeCommand("deshumidificador", "OFF");
  _server.send(200, "application/json", _metadata->getCommandResponseJSON("deshumidificador", "OFF", "MANUAL"));
}

void WebServerLib::handleDeshumAuto() {
  _actuators->executeCommand("deshumidificador", "AUTO");
  _server.send(200, "application/json", _metadata->getCommandResponseJSON("deshumidificador", "-", "AUTO"));
}

void WebServerLib::handleNotFound() {
  _server.send(404, "application/json", "{\"error\":\"Ruta no encontrada\"}");
}

String WebServerLib::getWebPage() {
  DeviceConfig config = _metadata->getConfig();
  
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Sistema de Riego IoT</title>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body { 
      font-family: 'Segoe UI', Tahoma, sans-serif; 
      background: linear-gradient(135deg, #1a1a2e 0%, #16213e 50%, #0f3460 100%);
      min-height: 100vh;
      color: #eee;
      padding: 20px;
    }
    .container { max-width: 1200px; margin: 0 auto; }
    h1 { 
      text-align: center; 
      margin-bottom: 30px; 
      color: #00d9ff;
      text-shadow: 0 0 10px rgba(0,217,255,0.5);
    }
    .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(280px, 1fr)); gap: 20px; }
    .card {
      background: rgba(255,255,255,0.1);
      border-radius: 15px;
      padding: 20px;
      backdrop-filter: blur(10px);
      border: 1px solid rgba(255,255,255,0.2);
    }
    .card h2 { color: #00d9ff; margin-bottom: 15px; font-size: 1.2em; }
    .sensor-value { font-size: 2.5em; font-weight: bold; text-align: center; margin: 15px 0; }
    .sensor-value.warning { color: #ffaa00; }
    .sensor-value.danger { color: #ff4444; }
    .sensor-value.success { color: #00ff88; }
    .status-label { text-align: center; padding: 8px 15px; border-radius: 20px; font-weight: bold; margin: 10px 0; }
    .status-on { background: #00ff88; color: #000; }
    .status-off { background: #ff4444; color: #fff; }
    .status-auto { background: #2196F3; color: #fff; }
    .status-manual { background: #ff9800; color: #000; }
    .btn-group { display: flex; gap: 10px; flex-wrap: wrap; justify-content: center; margin-top: 15px; }
    .btn { padding: 12px 20px; border: none; border-radius: 8px; cursor: pointer; font-weight: bold; flex: 1; min-width: 80px; }
    .btn-on { background: #00ff88; color: #000; }
    .btn-off { background: #ff4444; color: #fff; }
    .btn-auto { background: #2196F3; color: #fff; }
    .info-row { display: flex; justify-content: space-between; padding: 8px 0; border-bottom: 1px solid rgba(255,255,255,0.1); }
    .info-label { color: #aaa; }
    .info-value { font-weight: bold; }
    .refresh-info { text-align: center; color: #888; margin-top: 20px; }
  </style>
</head>
<body>
  <div class="container">
    <h1>🌱 Sistema de Riego Automatico IoT</h1>
    <div class="grid">
      <div class="card">
        <h2>🌡️ Temperatura</h2>
        <div class="sensor-value" id="temp">--</div>
        <div class="info-row"><span class="info-label">Umbral Max</span><span class="info-value">)rawliteral" + String(config.tempMax) + R"rawliteral( C</span></div>
        <div class="info-row"><span class="info-label">Umbral Min</span><span class="info-value">)rawliteral" + String(config.tempMin) + R"rawliteral( C</span></div>
      </div>
      <div class="card">
        <h2>💧 Humedad Ambiental</h2>
        <div class="sensor-value" id="humAmb">--</div>
        <div class="info-row"><span class="info-label">Umbral Max</span><span class="info-value">)rawliteral" + String(config.humedadMax) + R"rawliteral( %</span></div>
        <div class="info-row"><span class="info-label">Umbral Min</span><span class="info-value">)rawliteral" + String(config.humedadMin) + R"rawliteral( %</span></div>
      </div>
      <div class="card">
        <h2>🌍 Humedad del Suelo</h2>
        <div class="sensor-value" id="humSuelo">--</div>
      </div>
      <div class="card">
        <h2>☀️ Luminosidad</h2>
        <div class="sensor-value" id="ldr">--</div>
      </div>
      <div class="card">
        <h2>🚨 Movimiento</h2>
        <div id="movimiento" class="status-label status-off">SIN MOVIMIENTO</div>
        <div id="buzzerStatus" class="status-label">BUZZER: OFF</div>
      </div>
      <div class="card">
        <h2>🚿 Bomba de Riego</h2>
        <div id="bombaEstado" class="status-label status-off">OFF</div>
        <div id="bombaModo" class="status-label status-auto">MODO: AUTO</div>
        <div class="btn-group">
          <button class="btn btn-on" onclick="sendCmd('bomba','on')">ON</button>
          <button class="btn btn-off" onclick="sendCmd('bomba','off')">OFF</button>
          <button class="btn btn-auto" onclick="sendCmd('bomba','auto')">AUTO</button>
        </div>
      </div>
      <div class="card">
        <h2>🌀 Ventilador</h2>
        <div id="ventiladorEstado" class="status-label status-off">OFF</div>
        <div id="ventiladorModo" class="status-label status-auto">MODO: AUTO</div>
        <div class="btn-group">
          <button class="btn btn-on" onclick="sendCmd('ventilador','on')">ON</button>
          <button class="btn btn-off" onclick="sendCmd('ventilador','off')">OFF</button>
          <button class="btn btn-auto" onclick="sendCmd('ventilador','auto')">AUTO</button>
        </div>
      </div>
      <div class="card">
        <h2>💨 Deshumidificador</h2>
        <div id="deshumEstado" class="status-label status-off">OFF</div>
        <div id="deshumModo" class="status-label status-auto">MODO: AUTO</div>
        <div class="btn-group">
          <button class="btn btn-on" onclick="sendCmd('deshumidificador','on')">ON</button>
          <button class="btn btn-off" onclick="sendCmd('deshumidificador','off')">OFF</button>
          <button class="btn btn-auto" onclick="sendCmd('deshumidificador','auto')">AUTO</button>
        </div>
      </div>
      <div class="card">
        <h2>📡 Info Dispositivo</h2>
        <div class="info-row"><span class="info-label">IP</span><span class="info-value">)rawliteral" + WiFi.localIP().toString() + R"rawliteral(</span></div>
        <div class="info-row"><span class="info-label">MQTT</span><span class="info-value">)rawliteral" + config.mqttBroker + R"rawliteral(</span></div>
        <div style="margin-top:15px;">
          <a href="/api/device" target="_blank" style="color:#00d9ff;">Ver Metadatos JSON</a><br>
          <a href="/api/status" target="_blank" style="color:#00d9ff;">Ver Estado JSON</a>
        </div>
      </div>
    </div>
    <p class="refresh-info">Actualizacion automatica cada 2 segundos</p>
  </div>
  <script>
    function updateData() {
      fetch('/api/status').then(r=>r.json()).then(data=>{
        document.getElementById('temp').innerHTML = data.sensores.temperatura + ' C';
        document.getElementById('humAmb').innerHTML = data.sensores.humedad_ambiental + ' %';
        document.getElementById('humSuelo').innerHTML = data.sensores.humedad_suelo;
        document.getElementById('ldr').innerHTML = data.sensores.luminosidad;
        
        const movEl = document.getElementById('movimiento');
        movEl.className = 'status-label ' + (data.sensores.movimiento === 'DETECTADO' ? 'status-on' : 'status-off');
        movEl.innerHTML = data.sensores.movimiento === 'DETECTADO' ? 'MOVIMIENTO DETECTADO' : 'SIN MOVIMIENTO';
        
        const buzzerEl = document.getElementById('buzzerStatus');
        buzzerEl.innerHTML = 'BUZZER: ' + data.actuadores.buzzer.estado;
        buzzerEl.className = 'status-label ' + (data.actuadores.buzzer.estado === 'ON' ? 'status-on' : 'status-off');
        
        updateActuador('bomba', data.actuadores.bomba);
        updateActuador('ventilador', data.actuadores.ventilador);
        updateActuador('deshum', data.actuadores.deshumidificador);
        
        const sueloEl = document.getElementById('humSuelo');
        sueloEl.className = 'sensor-value ' + (data.sensores.humedad_suelo === 'SECO' ? 'warning' : 'success');
      }).catch(err=>console.log(err));
    }
    function updateActuador(nombre, data) {
      document.getElementById(nombre + 'Estado').innerHTML = data.estado;
      document.getElementById(nombre + 'Estado').className = 'status-label ' + (data.estado === 'ON' ? 'status-on' : 'status-off');
      document.getElementById(nombre + 'Modo').innerHTML = 'MODO: ' + data.modo;
      document.getElementById(nombre + 'Modo').className = 'status-label ' + (data.modo === 'AUTO' ? 'status-auto' : 'status-manual');
    }
    function sendCmd(actuador, comando) {
      fetch('/api/' + actuador + '/' + comando).then(r=>r.json()).then(()=>updateData()).catch(err=>console.log(err));
    }
    updateData();
    setInterval(updateData, 2000);
  </script>
</body>
</html>
)rawliteral";
  return html;
}

