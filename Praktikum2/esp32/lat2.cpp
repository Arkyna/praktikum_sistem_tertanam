#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>

// ── Konstanta pin ──────────────────────────────────────────
#define RAIN_SENSOR_PIN 34   // Sensor hujan  (AO)
#define LDR_SENSOR_PIN  35   // Sensor cahaya (AO)
#define SERVO_PIN       13   // Pin servo
#define LED_GREEN       27   // LED hijau
#define LED_RED         26   // LED merah

// ── Objek global ───────────────────────────────────────────
LiquidCrystal_I2C lcd(0x27, 16, 2);   // Alamat I²C 0x27 – ubah jika perlu
Servo atapServo;

void setup() {
  Serial.begin(115200);
  Wire.begin();               // SDA/SCL default ESP32 (21/22)

  // LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Sistem Atap OK");
  delay(2000);
  lcd.clear();

  // Pin I/O
  pinMode(RAIN_SENSOR_PIN, INPUT);
  pinMode(LDR_SENSOR_PIN,  INPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED,   OUTPUT);

  // Servo
  atapServo.attach(SERVO_PIN);
  atapServo.write(0);         // Mulai tertutup

  // Matikan LED
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED,   LOW);
}

void loop() {
  // ── Baca sensor ─────────────────────────────────────────
  int rainValue  = analogRead(RAIN_SENSOR_PIN);
  int lightValue = analogRead(LDR_SENSOR_PIN);

  Serial.print("Sensor Hujan : ");
  Serial.println(rainValue);
  Serial.print("Sensor Cahaya: ");
  Serial.println(lightValue);

  // Hujan jika ADC < 2000  (sesuaikan kalibrasi)
  bool hujan = rainValue < 2000;
  // Pagi jika terang → ADC cahaya < 2500
  bool pagi  = lightValue < 2500;

  // ── Baris 0: nilai hujan ───────────────────────────────
  lcd.setCursor(0, 0);
  lcd.print("Hujan: ");
  lcd.print(rainValue);
  lcd.print("   ");           // Hapus sisa digit lama

  // ── Baris 1: jam/grup waktu (tampil 1 s) ───────────────
  lcd.setCursor(0, 1);
  lcd.print("Waktu: ");
  lcd.print(pagi ? "PAGI " : "MALAM");
  lcd.print("   ");

  delay(1000);                // Refresh 1 detik

  // ── Baris 1: status atap & aksi servo/LED ──────────────
  lcd.setCursor(0, 1);
  if (hujan) {                            // Selalu tutup jika hujan
    lcd.print("Atap: TERTUTUP ");
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED,   HIGH);
    atapServo.write(0);                   // Tutup atap
  } else {
    if (pagi) {                           // Tidak hujan + pagi → buka
      lcd.print("Atap: TERBUKA  ");
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_RED,   LOW);
      atapServo.write(90);                // Buka atap (90°)
    } else {                              // Malam & tak hujan → tutup
      lcd.print("Atap: TERTUTUP ");
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_RED,   HIGH);
      atapServo.write(0);                 // Tutup atap
    }
  }

  delay(1000);                // Tunggu 1 detik sebelum baca ulang
}
