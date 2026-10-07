#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>

// Usamos el puerto Serial 2 del ESP32 (RX2 = Pin 16, TX2 = Pin 17)
HardwareSerial mySoftwareSerial(2);
DFRobotDFPlayerMini myDFPlayer;

// ACTUALIZADO: Cantidad de figuras y tus nuevos pines
const int numFiguras = 8;
const int pinesFiguras[numFiguras] = {26, 25, 33, 32, 13, 12, 14, 27};

// Arreglo para guardar la "memoria" del estado anterior de cada botón
bool estadoAnterior[numFiguras];

void setup() {
  Serial.begin(115200);
  // Inicializamos la comunicación con el DFPlayer a 9600 baudios
  mySoftwareSerial.begin(9600, SERIAL_8N1, 16, 17); 
  
  Serial.println("Iniciando Caja Sensorial...");

  // Configuramos los 8 pines
  for (int i = 0; i < numFiguras; i++) {
    pinMode(pinesFiguras[i], INPUT_PULLUP); // Activa los 3.3V internos
    estadoAnterior[i] = HIGH; // Asumimos que todos empiezan sueltos (HIGH)
  }

  // Verificamos que el DFPlayer esté conectado
  if (!myDFPlayer.begin(mySoftwareSerial)) {
    Serial.println("¡Error! Revisa conexiones del DFPlayer o la MicroSD");
    while (true); // Se detiene aquí si hay error
  }
  
  Serial.println("DFPlayer listo.");
  myDFPlayer.volume(20);  // Configura el volumen (rango de 0 a 30)
}

void loop() {
  // Escaneamos los 8 botones uno por uno
  for (int i = 0; i < numFiguras; i++) {
    // Leemos el pin actual
    bool estadoActual = digitalRead(pinesFiguras[i]);

    // LÓGICA DE MEMORIA:
    // Si el botón AHORA está presionado (LOW) y ANTES estaba suelto (HIGH)...
    if (estadoActual == LOW && estadoAnterior[i] == HIGH) {
      
      Serial.print("Figura insertada en ranura: ");
      Serial.println(i + 1);

      // Le decimos al DFPlayer que reproduzca la pista correspondiente (1 al 8)
      myDFPlayer.play(i + 1); 
      
      // Un pequeño retraso para evitar el "rebote" metálico del botón
      delay(50); 
    }

    // Actualizamos la memoria para la siguiente vuelta
    estadoAnterior[i] = estadoActual;
  }
  
  delay(10); // Pequeña pausa para no saturar el procesador del ESP32
}