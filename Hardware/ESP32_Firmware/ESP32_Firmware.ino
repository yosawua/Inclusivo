#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>
#include <WiFi.h>
#include <WebServer.h>

// --- CONFIGURACIÓN DE PINES Y AUDIO ---
HardwareSerial mySoftwareSerial(2);
DFRobotDFPlayerMini myDFPlayer;

const int numFiguras = 8;
const int pinesFiguras[numFiguras] = {26, 25, 33, 32, 13, 12, 14, 27};
bool estadoAnterior[numFiguras];

int ultimaPista = 0; // Para el log del dashboard

// --- CONFIGURACIÓN WIFI Y SERVIDOR WEB ---
const char* ssid = "Caja_Sensorial_AP";
const char* password = ""; // Sin contraseña para acceso rápido
WebServer server(80);

// --- CÓDIGO HTML + CSS + JAVASCRIPT ---
// Interfaz minimalista oscura con acentos neón
const char dashboard_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Inclusivio | Debugger</title>
    <style>
        body { background-color: #0d0d0d; color: #ececec; font-family: 'Consolas', 'Courier New', monospace; text-align: center; margin: 0; padding: 20px; }
        h1 { color: #00d2ff; text-transform: uppercase; letter-spacing: 2px; }
        .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(120px, 1fr)); gap: 15px; max-width: 600px; margin: 30px auto; }
        .slot { background: #1a1a1a; padding: 20px; border-radius: 8px; border: 2px solid #333; transition: all 0.2s ease-in-out; font-weight: bold; }
        .slot.active { border-color: #00d2ff; background: #002b36; box-shadow: 0 0 15px rgba(0, 210, 255, 0.5); color: #00d2ff; }
        #log { margin-top: 30px; padding: 15px; background: #111; border-left: 4px solid #ff0055; display: inline-block; text-align: left; }
        .status-dot { height: 10px; width: 10px; background-color: #00ff00; border-radius: 50%; display: inline-block; margin-right: 5px; box-shadow: 0 0 8px #00ff00;}
    </style>
</head>
<body>
    <h1><span class="status-dot"></span>Debugger Físico</h1>
    <p>Monitor de telemetría de Inclusivio en tiempo real</p>
    
    <div class="grid">
        <div class="slot" id="s0">Cuadrado</div>
        <div class="slot" id="s1">Círculo</div>
        <div class="slot" id="s2">Trapecio</div>
        <div class="slot" id="s3">Hexágono</div>
        <div class="slot" id="s4">Triángulo</div>
        <div class="slot" id="s5">Rectángulo</div>
        <div class="slot" id="s6">Rombo</div>
        <div class="slot" id="s7">Pentágono</div>
    </div>

    <div id="log">
        <strong>Sistema de Audio:</strong> <br>
        <span id="track-info">Esperando interacción...</span>
    </div>

    <script>
        // Fetch API para actualizar datos cada 300ms sin recargar la página
        setInterval(() => {
            fetch('/api/estado')
            .then(response => response.json())
            .then(data => {
                for (let i = 0; i < 8; i++) {
                    let elemento = document.getElementById('s' + i);
                    if (data.sensores[i] === 1) {
                        elemento.classList.add('active');
                    } else {
                        elemento.classList.remove('active');
                    }
                }
                if (data.pista > 0) {
                    document.getElementById('track-info').innerText = "Reproduciendo pista MP3: 00" + data.pista;
                }
            })
            .catch(error => console.error('Error de conexión:', error));
        }, 300);
    </script>
</body>
</html>
)rawliteral";

// --- ENDPOINTS DEL SERVIDOR ---
void handleRoot() {
  server.send(200, "text/html", dashboard_html);
}

void handleEstado() {
  // Construimos un JSON manual con el estado de los 8 pines
  String json = "{\"sensores\":[";
  for (int i = 0; i < numFiguras; i++) {
    // digitalRead es LOW (0) cuando está presionado. Lo invertimos para el JSON.
    int estado = (digitalRead(pinesFiguras[i]) == LOW) ? 1 : 0;
    json += String(estado);
    if (i < numFiguras - 1) json += ",";
  }
  json += "], \"pista\":" + String(ultimaPista) + "}";
  
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  mySoftwareSerial.begin(9600, SERIAL_8N1, 16, 17); 
  
  // 1. Configurar Pines
  for (int i = 0; i < numFiguras; i++) {
    pinMode(pinesFiguras[i], INPUT_PULL