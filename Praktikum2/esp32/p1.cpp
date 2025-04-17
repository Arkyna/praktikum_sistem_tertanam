#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

// ── Pin setup ───────────────────────────────────────────────
#define DHT_PIN      4     // Pastikan pin sesuai hardware
#define RAIN_SENSOR  35    // ADC pin sensor hujan

// ── Objects ────────────────────────────────────────────────
LiquidCrystal_I2C lcd(0x27, 16, 2);   // I2C address 0x27, 16×2
DHT dht(DHT_PIN, DHT11);              // Ganti DHT11→DHT22 jika perlu

void setup() {
  Serial.begin(115200);

  Wire.begin();                       // I²C init
  dht.begin();                        // DHT init
  lcd.begin(16, 2);
  lcd.backlight();

  pinMode(RAIN_SENSOR, INPUT);
  analogReadResolution(12);           // 0‑4095 (ESP32)

  lcd.setCursor(0, 0);
  lcd.print("System Loading...");
  delay(3000);
  lcd.clear();
}

void loop() {
  // ── DHT11 ────────────────────────────────────────────────
  float humidity    = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("⚠️  Gagal membaca sensor DHT11!");
    lcd.setCursor(0, 0);
    lcd.print("DHT11 Error!     ");
  } else {
    Serial.print("Temp: ");
    Serial.print(temperature);
    Serial.print(" °C, Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(temperature, 1);
    lcd.print("C ");

    lcd.setCursor(8, 0);
    lcd.print("H:");
    lcd.print(humidity, 1);
    lcd.print("% ");
  }

  // ── Rain sensor ──────────────────────────────────────────
  int rainValueRaw = analogRead(RAIN_SENSOR);
  Serial.print("Raw Rain Sensor: ");
  Serial.println(rainValueRaw);

  // Balikkan: 0 = basah, 100 = kering
  int rainValue = map(rainValueRaw, 4095, 0, 0, 100);

  Serial.print("Rain Level: ");
  Serial.print(rainValue);
  Serial.println(" %");

  lcd.setCursor(0, 1);
  lcd.print("Rain:");
  lcd.print(rainValue);
  lcd.print("%   ");

  // ── Delay ────────────────────────────────────────────────
  delay(3000);   // Baca DHT agak jarang biar stabil
}
