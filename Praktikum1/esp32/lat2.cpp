#include <ESP32Servo.h>

// ── Pin mapping ─────────────────────────────────────────────
#define SOIL_SENSOR_PIN 35   // ADC pin untuk sensor kelembapan tanah
#define LED_LEMBAB     26    // LED hijau  → tanah lembap
#define LED_KERING     27    // LED merah  → tanah kering
#define SERVO_PIN       4    // Servo penggerak atap

Servo atapServo;

// ── Threshold ADC (kalibrasi sesuai sensor & tegangan) ─────
const int thresholdKering  = 3200;  // >3200 ≈ kering
const int thresholdLembab  = 2500;  // <2500 ≈ lembap

void setup() {
  Serial.begin(115200);

  pinMode(LED_LEMBAB, OUTPUT);
  pinMode(LED_KERING, OUTPUT);
  atapServo.attach(SERVO_PIN);
}

void loop() {
  int soilMoisture = analogRead(SOIL_SENSOR_PIN);   // Baca ADC
  Serial.print("Kelembapan Tanah: ");
  Serial.println(soilMoisture);

  // ── Kondisi tanah kering ────────────────────────────────
  if (soilMoisture > thresholdKering) {
    digitalWrite(LED_KERING, HIGH);
    digitalWrite(LED_LEMBAB, LOW);
    Serial.println("⚠️  Tanah Kering! Membuka atap…");
    atapServo.write(90);                         // Atap terbuka penuh
  }

  // ── Kondisi tanah lembap ────────────────────────────────
  else if (soilMoisture < thresholdLembab) {
    digitalWrite(LED_KERING, LOW);
    digitalWrite(LED_LEMBAB, HIGH);
    Serial.println("✅ Tanah Lembap! Menutup atap…");
    atapServo.write(0);                          // Atap tertutup
  }

  // ── Kondisi normal (antara dua ambang) ──────────────────
  else {
    digitalWrite(LED_KERING, LOW);
    digitalWrite(LED_LEMBAB, LOW);
    Serial.println("➖ Tanah dalam kondisi normal.");
    atapServo.write(45);                         // Atap setengah terbuka
  }

  delay(2000);  // Biar MCU tak spam, 2 detik sekali cukup
}
