#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <time.h>

// Definisi Pin untuk LED dan Sensor
#define LED_RED       26    // LED Merah untuk kondisi gelap
#define LED_GREEN     27    // LED Hijau untuk kondisi terang
#define LED_YELLOW    25    // LED Kuning untuk sensor hujan

#define LDR_SENSOR_PIN   34   // Sensor LDR (analog input)
#define RAIN_SENSOR_PIN  35   // Sensor hujan (digital input)
#define LDR_THRESHOLD  2500  // Sesuaikan nilai threshold

// Kredensial Firebase & WiFi
#define API_KEY           "XXXX" //Sesuaikan isinya
#define USER_EMAIL        "XXXX"
#define USER_PASSWORD     "XXXX"
#define DATABASE_URL      "XXXX"

#define WIFI_SSID         "XXXX"
#define WIFI_PASSWORD     "XXXX"

// Objek Firebase
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void setup() {
  Serial.begin(115200);

  // Inisialisasi LED
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);

  // Inisialisasi sensor
  pinMode(LDR_SENSOR_PIN, INPUT);
  pinMode(RAIN_SENSOR_PIN, INPUT);

  // Hubungkan ke WiFi
  Serial.print("Menghubungkan ke WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("WiFi terhubung. IP: ");
  Serial.println(WiFi.localIP());

  // Konfigurasi Firebase
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Inisialisasi waktu melalui NTP
  configTime(7 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  time_t now = time(nullptr);
  while (now < 100000) {
    delay(100);
    now = time(nullptr);
  }
}

void loop() {
  // Baca sensor LDR
  int ldrValue = analogRead(LDR_SENSOR_PIN);
  bool isDark = ldrValue > LDR_THRESHOLD;
  String lightCondition = isDark ? "Gelap" : "Terang";

  // Baca sensor hujan (LOW = hujan, HIGH = tidak hujan)
  bool isRaining = digitalRead(RAIN_SENSOR_PIN) == LOW;

  // Kontrol LED berdasarkan kondisi
  digitalWrite(LED_RED, isDark ? HIGH : LOW);
  digitalWrite(LED_GREEN, isDark ? LOW : HIGH);
  digitalWrite(LED_YELLOW, isRaining ? HIGH : LOW);

  // Dapatkan timestamp (format: YYYY-MM-DD HH:MM:SS)
  time_t nowTime = time(nullptr);
  struct tm timeinfo;
  localtime_r(&nowTime, &timeinfo);
  char timeStr[20];
  strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);

  // Tampilkan data ke Serial Monitor
  Serial.print("LDR Value: ");
  Serial.print(ldrValue);
  Serial.print(" | Kondisi: ");
  Serial.print(lightCondition);
  Serial.print(" | Hujan: ");
  Serial.print(isRaining ? "Ya" : "Tidak");
  Serial.print(" | Timestamp: ");
  Serial.println(timeStr);

  // Siapkan data JSON untuk dikirim ke Firebase
  FirebaseJson json;
  json.set("condition", lightCondition);
  json.set("timestamp", timeStr);
  json.set("ldr_value", ldrValue);
  json.set("raining", isRaining);

  // Kirim data ke Firebase Realtime Database di path /EnvironmentData
  if (Firebase.RTDB.setJSON(&fbdo, "/EnvironmentData", &json)) {
    Serial.println("Data berhasil dikirim ke Firebase");
  } else {
    Serial.print("Gagal mengirim data: ");
    Serial.println(fbdo.errorReason());
  }
  
  delay(1500);  // Update setiap 1.5 detik
}