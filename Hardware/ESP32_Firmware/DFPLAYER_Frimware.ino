// Prueba simple: manda ordenes de reproducir sin esperar respuesta
// TX modulo -> GPIO16 | RX modulo -> GPIO17 (con 1k) | GND comun | VCC 5V
// SD: carpeta /mp3/ con 0001.mp3 ... 0009.mp3

#define MP3_RX 16
#define MP3_TX 17

HardwareSerial mp3Serial(2);

void sendCmd(uint8_t cmd, uint8_t p1, uint8_t p2) {
  uint16_t sum = 0xFF + 0x06 + cmd + 0x00 + p1 + p2;
  uint16_t chk = 0xFFFF - sum + 1;
  uint8_t frame[10] = {0x7E, 0xFF, 0x06, cmd, 0x00, p1, p2,
                       (uint8_t)(chk >> 8), (uint8_t)(chk & 0xFF), 0xEF};
  mp3Serial.write(frame, 10);
  Serial.print("Enviado: ");
  for (int i = 0; i < 10; i++) Serial.printf("%02X ", frame[i]);
  Serial.println();
}

void dumpRx(unsigned long ms) {
  unsigned long t = millis();
  int n = 0;
  while (millis() - t < ms) {
    while (mp3Serial.available()) {
      Serial.printf("%02X ", mp3Serial.read());
      n++;
    }
  }
  if (n == 0) Serial.print("(sin respuesta, es normal en algunos clones)");
  Serial.println();
}

int pista = 1;

void setup() {
  Serial.begin(115200);
  mp3Serial.begin(9600, SERIAL_8N1, MP3_RX, MP3_TX);
  delay(2000);

  Serial.println("Reset...");
  sendCmd(0x0C, 0x00, 0x00);
  dumpRx(2500);

  Serial.println("Volumen 20...");
  sendCmd(0x06, 0x00, 20);
  dumpRx(500);
}

void loop() {
  Serial.printf("Reproduciendo pista %d (/mp3/000%d.mp3)\n", pista, pista);
  sendCmd(0x03, 0x00, pista);   // en vez de sendCmd(0x12, 0x00, pista);
  dumpRx(5000);
  pista = (pista % 9) + 1;
}
