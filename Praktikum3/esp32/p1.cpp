#include <WiFi.h>
#include <Firebase_ESP_Client.h>

// Definisi pin untuk indikator dan sensor
#define LED_MERAH   25
#define LED_KUNING  26
#define LED_HIJAU   27
#define BUZZER      33

// Sensor HC-SR04
const int trigPin = 16;
const int echoPin = 17;
long duration;
float distance;

// Kredensial Firebase dan akun user (non anon)
#define API_KEY           "xxxx"  // Ganti dengan API Key Firebase
#define USER_EMAIL        "xxxx"  // Ganti dengan email user
#define USER_PASSWORD     "xxxx"  // Ganti dengan password user
#define DATABASE_URL      "xxxx"  // Ganti dengan URL database

// Kredensial WiFi
#define WIFI_SSID         "xxxx"  // Ganti dengan nama WiFi
#define WIFI_PASSWORD     "xxxx"  // Ganti dengan password WiFi

// Objek Firebase
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void setup() {
  Serial.begin(115200);
  
  // Inisialisasi pin
  pinMode(LED_MERAH, OUTPUT);
  pinMode(LED_KUNING, OUTPUT);
  pinMode(LED_HIJAU, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  // Pastikan sensor trigger dalam keadaan LOW
  digitalWrite(trigPin, LOW);
  
  // Hubungkan ke WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected to WiFi. IP: ");
  Serial.println(WiFi.localIP());
  
  // Konfigurasi Firebase dengan autentikasi akun pengguna
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  
  // Inisialisasi Firebase
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  // Reset indikator tiap iterasi agar gak ada status yang tertinggal
  digitalWrite(LED_MERAH, LOW);
  digitalWrite(LED_KUNING, LOW);
  digitalWrite(LED_HIJAU, LOW);
  digitalWrite(BUZZER, LOW);
  
  // Kirim pulsa ke sensor HC-SR04
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Baca durasi pulsa balik dan hitung jarak (cm)
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;
  
  Serial.print("Jarak: ");
  Serial.print(distance);
  Serial.println(" cm");
  
  // Kirim data jarak ke Firebase Realtime Database
  if (Firebase.RTDB.setFloat(&fbdo, "/WaterDistance", distance)) {
    Serial.println("Data dikirim ke Firebase");
  } else {
    Serial.print("Gagal mengirim data: ");
    Serial.println(fbdo.errorReason());
  }
  
  // Logika indikator:
  // Jarak < 5 cm: LED merah + buzzer aktif
  if (distance < 5) {
    digitalWrite(LED_MERAH, HIGH);
    digitalWrite(BUZZER, HIGH);
  } 
  // Jarak 5 - 10 cm: LED kuning berkedip (peringatan)
  else if (distance >= 5 && distance < 10) {
    digitalWrite(LED_KUNING, HIGH);
    delay(100);
    digitalWrite(LED_KUNING, LOW);
    delay(100);
  } 
  // Jarak ≥ 10 cm: LED hijau aktif (aman)
  else {
    digitalWrite(LED_HIJAU, HIGH);
  }
  
  delay(500);  // Sedikit delay
}
