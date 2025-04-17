#define SOIL_SENSOR_PIN 35  // Sensor kelembapan di GPIO35
#define LED_LEMBAB     26   // LED hijau  (tanah lembab)
#define LED_KERING     27   // LED merah  (tanah kering)

void setup() {
  Serial.begin(115200);
  pinMode(LED_LEMBAB, OUTPUT);
  pinMode(LED_KERING, OUTPUT);
}

void loop() {
  int soilMoisture = analogRead(SOIL_SENSOR_PIN);   // Baca kelembapan tanah
  Serial.print("Kelembapan Tanah: ");
  Serial.println(soilMoisture);                     // Tampilkan di Serial Monitor

  int thresholdKering = 3000; // ADC > 3000  ⇒ tanah kering
  int thresholdLembab = 2000; // ADC < 2000  ⇒ tanah lembab

  if (soilMoisture > thresholdKering) {
    digitalWrite(LED_KERING, HIGH);  // LED merah nyala
    digitalWrite(LED_LEMBAB, LOW);
    Serial.println("⚠️  Tanah Kering!");
  } else if (soilMoisture < thresholdLembab) {
    digitalWrite(LED_KERING, LOW);
    digitalWrite(LED_LEMBAB, HIGH);  // LED hijau nyala
    Serial.println("✅ Tanah Lembab!");
  } else {
    digitalWrite(LED_KERING, LOW);
    digitalWrite(LED_LEMBAB, LOW);
    Serial.println("Tanah dalam kondisi normal.");
  }

  delay(2000); // Jeda 2 s sebelum baca ulang
}
