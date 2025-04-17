#define LDR 32    // Pin sensor LDR
#define LED 4     // Pin LED

void setup() {
  Serial.begin(115200);          // Pakai baud tinggi di ESP32
  pinMode(LED, OUTPUT);
}

void loop() {
  int nilai = analogRead(LDR);   // Baca intensitas cahaya dari LDR
  Serial.print("Nilai ADC: ");
  Serial.println(nilai);         // Pantau di Serial Monitor

  if (nilai >= 3000) {           // Atur threshold sesuai hasil uji
    digitalWrite(LED, HIGH);
  } else {
    digitalWrite(LED, LOW);
  }

  delay(500);                    // Jeda 500 ms biar nggak keburu‑buru
}
