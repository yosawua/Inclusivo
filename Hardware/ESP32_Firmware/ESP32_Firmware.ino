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

int ultimaPista = 9; // Arrancamos mostrando la pista de bienvenida en el log

// --- CONFIGURACIÓN WIFI Y SERVIDOR WEB ---
const char* ssid = "Caja_Sensorial_AP";
const char* password = ""; // Sin contraseña para acceso rápido
WebServer server(80);

// --- CÓDIGO HTML + CSS + JAVASCRIPT ---
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
  String json = "{\"sensores\":[";
  for (int i = 0; i < numFiguras; i++) {
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
    pinMode(pinesFiguras[i], INPUT_PULLUP);
    estadoAnterior[i] = HIGH;
  }

  // 2. Levantar Red WiFi (Access Point)
  Serial.println("\nIniciando AP WiFi...");
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Servidor web iniciado. Conéctate a la red 'Caja_Sensorial_AP' y abre en el navegador: http://");
  Serial.println(IP);

  // 3. Configurar Rutas del Servidor
  server.on("/", handleRoot);
  server.on("/api/estado", handleEstado);
  server.begin();

  // 4. Inicializar DFPlayer y Reproducir Bienvenida
  if (!myDFPlayer.begin(mySoftwareSerial)) {
    Serial.println("Error de DFPlayer.");
  } else {
    myDFPlayer.volume(22); // Volumen ideal (0 a 30)
    delay(1000); // Pequeña pausa para asegurar arranque del módulo
    myDFPlayer.play(9); // Reproduce el archivo 009.mp3 de bienvenida al encender
  }
}

void loop() {
  server.handleClient();

  for (int i = 0; i < numFiguras; i++) {
    bool estadoActual = digitalRead(pinesFiguras[i]);

    if (estadoActual == LOW && estadoAnterior[i] == HIGH) {
      Serial.print("Figura detectada: ");
      Serial.println(i + 1);
      
      myDFPlayer.play(i + 1);
      ultimaPista = i + 1; 
      
      delay(50); // Debounce
    }
    estadoAnterior[i] = estadoActual;
  }
}