#define FIRE_SENSOR_PIN 34   // Pin sensor api  (analog)
#define GAS_SENSOR_PIN  35   // Pin sensor gas/asap (analog)
#define BUZZER_PIN      18   // Pin buzzer
#define LED_PIN         17   // Pin LED merah

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN,     OUTPUT);
  pinMode(FIRE_SENSOR_PIN, INPUT);
  pinMode(GAS_SENSOR_PIN,  INPUT);
  Serial.begin(115200);
}

void loop() {
  int fireValue = analogRead(FIRE_SENSOR_PIN);
  int gasValue  = analogRead(GAS_SENSOR_PIN);

  Serial.print("Fire Sensor: ");
  Serial.println(fireValue);
  Serial.print("Gas Sensor:  ");
  Serial.println(gasValue);

  // ── Threshold: <1000 = api terdeteksi, >2000 = gas pekat ──
  if (fireValue < 1000 || gasValue > 2000) {
    digitalWrite(BUZZER_PIN, HIGH);      // Bunyikan buzzer

    // Blink LED 5× biar dramatis
    for (int i = 0; i < 5; i++) {
      digitalWrite(LED_PIN, HIGH);
      delay(200);
      digitalWrite(LED_PIN, LOW);
      delay(200);
    }
  } else {
    digitalWrite(BUZZER_PIN, LOW);       // Sunyi kembali
    digitalWrite(LED_PIN, LOW);
  }

  delay(1000);                           // Update tiap detik
}
