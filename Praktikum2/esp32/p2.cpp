#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define GAS_SENSOR_PIN   36   // MQ‑6 AO → VP (ADC1_CH0)
#define FLAME_SENSOR_PIN 34   // KY‑026 DO → GPIO34

// LCD I2C (alamat default 0x27; ubah jika modul berbeda)
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(115200);

  // ── LCD init ────────────────────────────────────────────
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Sistem Pendeteksi");
  lcd.setCursor(4, 1);
  lcd.print("Kebakaran");
  delay(2000);                // Splash screen 2 s

  // ── Sensor pin mode ─────────────────────────────────────
  pinMode(GAS_SENSOR_PIN,   INPUT);
  pinMode(FLAME_SENSOR_PIN, INPUT);

  Serial.println("Sistem pendeteksi kebakaran dimulai...");
}

void loop() {
  // ── Baca sensor ─────────────────────────────────────────
  int  gasLevel      = analogRead(GAS_SENSOR_PIN);          // 0‑4095
  bool flameDetected = digitalRead(FLAME_SENSOR_PIN) == LOW;// LOW = api

  // ── Serial monitor ─────────────────────────────────────
  Serial.print("Gas: ");
  Serial.print(gasLevel);
  Serial.print("  Flame: ");
  Serial.println(flameDetected ? "YES" : "NO");

  // ── Tampilkan di LCD 16×2 ──────────────────────────────
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Gas: ");
  lcd.print(gasLevel);

  lcd.setCursor(0, 1);
  lcd.print("Flame: ");
  lcd.print(flameDetected ? "Y" : "N");

  delay(2000);  // Refresh setiap 2 detik
}
