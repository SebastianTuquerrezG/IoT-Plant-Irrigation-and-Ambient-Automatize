#include "WebServerLib.h"

WebServerLib::WebServerLib() : _server(80) {
  _sensors = nullptr;
  _actuators = nullptr;
  _metadata = nullptr;
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
  
  // API REST - Deshumidificador
  _server.on("/api/deshumidificador/on", HTTP_GET, [this]() { this->handleDeshumOn(); });
  _server.on("/api/deshumidificador/off", HTTP_GET, [this]() { this->handleDeshumOff(); });
  _server.on("/api/deshumidificador/auto", HTTP_GET, [this]() { this->handleDeshumAuto(); });
  
  // API REST - Umbrales
  _server.on("/api/umbrales", HTTP_GET, [this]() { this->handleGetUmbrales(); });
  _server.on("/api/umbral/set", HTTP_GET, [this]() { this->handleSetUmbral(); });
  
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

// ========== HANDLERS BOMBA ==========
void WebServerLib::handleBombaOn() {
  _actuators->setBombaManual(true);
  _actuators->setBomba(true);
  Serial.println("[WebServer] Bomba -> ON (manual)");
  _server.send(200, "application/json", "{\"success\":true,\"actuador\":\"bomba\",\"estado\":\"ON\",\"modo\":\"MANUAL\"}");
}

void WebServerLib::handleBombaOff() {
  _actuators->setBombaManual(true);
  _actuators->setBomba(false);
  Serial.println("[WebServer] Bomba -> OFF (manual)");
  _server.send(200, "application/json", "{\"success\":true,\"actuador\":\"bomba\",\"estado\":\"OFF\",\"modo\":\"MANUAL\"}");
}

void WebServerLib::handleBombaAuto() {
  _actuators->setBombaManual(false);
  Serial.println("[WebServer] Bomba -> AUTO");
  _server.send(200, "application/json", "{\"success\":true,\"actuador\":\"bomba\",\"modo\":\"AUTO\"}");
}

// ========== HANDLERS DESHUMIDIFICADOR ==========
void WebServerLib::handleDeshumOn() {
  _actuators->setDeshumidificadorManual(true);
  _actuators->setDeshumidificador(true);
  Serial.println("[WebServer] Deshumidificador -> ON (manual)");
  _server.send(200, "application/json", "{\"success\":true,\"actuador\":\"deshumidificador\",\"estado\":\"ON\",\"modo\":\"MANUAL\"}");
}

void WebServerLib::handleDeshumOff() {
  _actuators->setDeshumidificadorManual(true);
  _actuators->setDeshumidificador(false);
  Serial.println("[WebServer] Deshumidificador -> OFF (manual)");
  _server.send(200, "application/json", "{\"success\":true,\"actuador\":\"deshumidificador\",\"estado\":\"OFF\",\"modo\":\"MANUAL\"}");
}

void WebServerLib::handleDeshumAuto() {
  _actuators->setDeshumidificadorManual(false);
  Serial.println("[WebServer] Deshumidificador -> AUTO");
  _server.send(200, "application/json", "{\"success\":true,\"actuador\":\"deshumidificador\",\"modo\":\"AUTO\"}");
}

// ========== HANDLERS UMBRALES ==========
void WebServerLib::handleGetUmbrales() {
  _server.send(200, "application/json", _metadata->getUmbralesJSON());
}

void WebServerLib::handleSetUmbral() {
  String tipo = _server.arg("tipo");
  String valorStr = _server.arg("valor");
  
  if (tipo.length() == 0 || valorStr.length() == 0) {
    _server.send(400, "application/json", "{\"error\":\"Faltan parametros: tipo y valor\"}");
    return;
  }
  
  float valor = valorStr.toFloat();
  
  if (tipo == "humedad_max") {
    _metadata->setHumedadMax(valor);
  } else if (tipo == "humedad_min") {
    _metadata->setHumedadMin(valor);
  } else if (tipo == "temp_max") {
    _metadata->setTempMax(valor);
  } else if (tipo == "temp_min") {
    _metadata->setTempMin(valor);
  } else {
    _server.send(400, "application/json", "{\"error\":\"Tipo de umbral no valido\"}");
    return;
  }
  
  String response = "{\"success\":true,\"tipo\":\"" + tipo + "\",\"valor\":" + String(valor, 1) + "}";
  _server.send(200, "application/json", response);
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
    .btn { padding: 12px 20px; border: none; border-radius: 8px; cursor: pointer; font-weight: bold; flex: 1; min-width: 80px; transition: transform 0.1s; }
    .btn:active { transform: scale(0.95); }
    .btn-on { background: #00ff88; color: #000; }
    .btn-off { background: #ff4444; color: #fff; }
    .btn-auto { background: #2196F3; color: #fff; }
    .info-row { display: flex; justify-content: space-between; padding: 8px 0; border-bottom: 1px solid rgba(255,255,255,0.1); align-items: center; }
    .info-label { color: #aaa; }
    .info-value { font-weight: bold; }
    .refresh-info { text-align: center; color: #888; margin-top: 20px; }
    .umbral-input { 
      width: 70px; 
      padding: 5px 8px; 
      border: 1px solid #00d9ff; 
      border-radius: 5px; 
      background: rgba(0,0,0,0.3); 
      color: #fff; 
      text-align: center;
      font-size: 1em;
    }
    .umbral-input:focus { outline: none; border-color: #00ff88; }
    .btn-save { 
      padding: 5px 10px; 
      background: #00d9ff; 
      color: #000; 
      border: none; 
      border-radius: 5px; 
      cursor: pointer; 
      margin-left: 5px;
      font-size: 0.9em;
    }
    .btn-save:hover { background: #00ff88; }
  </style>
</head>
<body>
  <div class="container">
    <h1>🌱 Sistema de Riego Automatico IoT</h1>
    <div class="grid">
      <!-- Temperatura -->
      <div class="card">
        <h2>🌡️ Temperatura</h2>
        <div class="sensor-value" id="temp">--</div>
        <div class="info-row">
          <span class="info-label">Solo monitoreo</span>
          <span class="info-value" style="color:#888;">Sin actuador</span>
        </div>
      </div>

      <!-- Humedad Ambiental con umbrales editables -->
      <div class="card">
        <h2>💧 Humedad Ambiental</h2>
        <div class="sensor-value" id="humAmb">--</div>
        <div class="info-row">
          <span class="info-label">Umbral Max</span>
          <span>
            <input type="number" id="umbral_hum_max" class="umbral-input" value=")rawliteral" + String(config.humedadMax, 0) + R"rawliteral(">
            <button class="btn-save" onclick="setUmbral('humedad_max', 'umbral_hum_max')">OK</button>
          </span>
        </div>
        <div class="info-row">
          <span class="info-label">Umbral Min</span>
          <span>
            <input type="number" id="umbral_hum_min" class="umbral-input" value=")rawliteral" + String(config.humedadMin, 0) + R"rawliteral(">
            <button class="btn-save" onclick="setUmbral('humedad_min', 'umbral_hum_min')">OK</button>
          </span>
        </div>
      </div>

      <!-- Humedad del Suelo -->
      <div class="card">
        <h2>🌍 Humedad del Suelo</h2>
        <div class="sensor-value" id="humSuelo">--</div>
      </div>

      <!-- Luminosidad -->
      <div class="card">
        <h2>☀️ Luminosidad</h2>
        <div class="sensor-value" id="ldr">--</div>
      </div>

      <!-- Movimiento -->
      <div class="card">
        <h2>🚨 Movimiento</h2>
        <div id="movimiento" class="status-label status-off">SIN MOVIMIENTO</div>
        <div id="buzzerStatus" class="status-label">BUZZER: OFF</div>
      </div>

      <!-- Bomba de Riego -->
      <div class="card">
        <h2>🚿 Bomba de Riego</h2>
        <div id="bombaEstado" class="status-label status-off">OFF</div>
        <div id="bombaModo" class="status-label status-auto">MODO: AUTO</div>
        <div class="btn-group">
          <button class="btn btn-on" onclick="controlActuador('bomba','on')">ENCENDER</button>
          <button class="btn btn-off" onclick="controlActuador('bomba','off')">APAGAR</button>
          <button class="btn btn-auto" onclick="controlActuador('bomba','auto')">AUTO</button>
        </div>
      </div>

      <!-- Deshumidificador -->
      <div class="card">
        <h2>💨 Deshumidificador</h2>
        <div id="deshumEstado" class="status-label status-off">OFF</div>
        <div id="deshumModo" class="status-label status-auto">MODO: AUTO</div>
        <div class="btn-group">
          <button class="btn btn-on" onclick="controlActuador('deshumidificador','on')">ENCENDER</button>
          <button class="btn btn-off" onclick="controlActuador('deshumidificador','off')">APAGAR</button>
          <button class="btn btn-auto" onclick="controlActuador('deshumidificador','auto')">AUTO</button>
        </div>
      </div>

      <!-- Info del Dispositivo -->
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
    // Actualizar datos de sensores y actuadores
    function updateData() {
      fetch('/api/status')
        .then(r => r.json())
        .then(data => {
          // Sensores
          document.getElementById('temp').innerHTML = data.sensores.temperatura + ' C';
          document.getElementById('humAmb').innerHTML = data.sensores.humedad_ambiental + ' %';
          document.getElementById('humSuelo').innerHTML = data.sensores.humedad_suelo;
          document.getElementById('ldr').innerHTML = data.sensores.luminosidad;
          
          // Estado del suelo
          const sueloEl = document.getElementById('humSuelo');
          sueloEl.className = 'sensor-value ' + (data.sensores.humedad_suelo === 'SECO' ? 'warning' : 'success');
          
          // Movimiento
          const movEl = document.getElementById('movimiento');
          if (data.sensores.movimiento === 'DETECTADO') {
            movEl.className = 'status-label status-on';
            movEl.innerHTML = 'MOVIMIENTO DETECTADO';
          } else {
            movEl.className = 'status-label status-off';
            movEl.innerHTML = 'SIN MOVIMIENTO';
          }
          
          // Buzzer
          const buzzerEl = document.getElementById('buzzerStatus');
          buzzerEl.innerHTML = 'BUZZER: ' + data.actuadores.buzzer.estado;
          buzzerEl.className = 'status-label ' + (data.actuadores.buzzer.estado === 'ON' ? 'status-on' : 'status-off');
          
          // Bomba
          updateActuadorUI('bomba', data.actuadores.bomba);
          
          // Deshumidificador
          updateActuadorUI('deshum', data.actuadores.deshumidificador);
          
          // Actualizar inputs de umbrales si existen en la respuesta
          if (data.umbrales) {
            document.getElementById('umbral_hum_max').value = data.umbrales.humedad_max;
            document.getElementById('umbral_hum_min').value = data.umbrales.humedad_min;
          }
        })
        .catch(err => console.log('Error:', err));
    }
    
    // Actualizar UI de un actuador
    function updateActuadorUI(nombre, data) {
      const estadoEl = document.getElementById(nombre + 'Estado');
      const modoEl = document.getElementById(nombre + 'Modo');
      
      estadoEl.innerHTML = data.estado;
      estadoEl.className = 'status-label ' + (data.estado === 'ON' ? 'status-on' : 'status-off');
      
      modoEl.innerHTML = 'MODO: ' + data.modo;
      modoEl.className = 'status-label ' + (data.modo === 'AUTO' ? 'status-auto' : 'status-manual');
    }
    
    // Enviar comando a actuador
    function controlActuador(actuador, comando) {
      console.log('Enviando comando:', actuador, comando);
      fetch('/api/' + actuador + '/' + comando)
        .then(r => r.json())
        .then(data => {
          console.log('Respuesta:', data);
          // Actualizar inmediatamente
          updateData();
        })
        .catch(err => console.log('Error:', err));
    }
    
    // Establecer umbral
    function setUmbral(tipo, inputId) {
      const valor = document.getElementById(inputId).value;
      fetch('/api/umbral/set?tipo=' + tipo + '&valor=' + valor)
        .then(r => r.json())
        .then(data => {
          console.log('Umbral actualizado:', data);
          alert('Umbral ' + tipo + ' actualizado a ' + valor);
        })
        .catch(err => {
          console.log('Error:', err);
          alert('Error al actualizar umbral');
        });
    }
    
    // Iniciar actualizaciones
    updateData();
    setInterval(updateData, 2000);
  </script>
</body>
</html>
)rawliteral";
  return html;
}
