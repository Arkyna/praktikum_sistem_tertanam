#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include "DHT.h"
#include <Preferences.h>

Preferences prefs;

const char* ssid = "xxx";
const char* password = "xxx";
#define BOT_TOKEN "xxx"
#define CHAT_ID "xxx"

#define LDR_PIN 34
#define RAIN_PIN 35
#define DHTPIN 4
#define DHTTYPE DHT22
#define BUZZER_PIN 26
#define FAN_IN1 14
#define FAN_IN2 27
#define MOTOR_IN1 12
#define MOTOR_IN2 13
#define MOTOR_ENB 25

#define PWM_CHANNEL 0
#define PWM_FREQ 5000
#define PWM_RESOLUTION 8

float TEMP_THRESHOLD = 28.0;
float HUM_THRESHOLD = 75.0;
int LDR_THRESHOLD = 2100;

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);
DHT dht(DHTPIN, DHTTYPE);

bool jemuranKeluar = false;
bool modeOtomatis = true;
bool debugMode = false;
unsigned long lastMsgTime = 0;
unsigned long lastDebugTime = 0;
unsigned long lastSensorTime = 0;

const int botRequestDelay = 1000;
const unsigned long sensorInterval = 1000;

void setup() {
  prefs.begin("flags", false);
  bool justReset = prefs.getBool("justReset", false);
  if (justReset) {
    prefs.putBool("justReset", false);
    Serial.println("[INFO] Boot setelah reset, abaikan command pertama.");
    delay(3000);
  }
  prefs.end();

  Serial.begin(115200);
  dht.begin();

  pinMode(LDR_PIN, INPUT);
  pinMode(RAIN_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(FAN_IN1, OUTPUT);
  pinMode(FAN_IN2, OUTPUT);
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(MOTOR_ENB, OUTPUT);

  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(MOTOR_ENB, PWM_CHANNEL);

  digitalWrite(BUZZER_PIN, LOW);
  stopFan();
  stopMotor();

  WiFi.begin(ssid, password);
  client.setInsecure();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
    delay(1000);
    return;
  }

  if (millis() - lastMsgTime > botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleTelegramMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastMsgTime = millis();
  }

  if (millis() - lastSensorTime >= sensorInterval) {
    lastSensorTime = millis();
    handleSensorLogic();
  }
}

void bukaJemuran(String alasan = "", int speed = 128, int runTime = 200) {
  ledcWrite(PWM_CHANNEL, speed);
  digitalWrite(MOTOR_IN1, HIGH);
  digitalWrite(MOTOR_IN2, LOW);
  delay(runTime);
  stopMotor();
  jemuranKeluar = true;
  if (alasan != "") bot.sendMessage(CHAT_ID, "\u2600\ufe0f Dibuka: " + alasan, "");
}

void tutupJemuran(String alasan = "", int speed = 125, int runTime = 173) {
  ledcWrite(PWM_CHANNEL, speed);
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, HIGH);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(runTime);
  stopMotor();
  digitalWrite(BUZZER_PIN, LOW);
  jemuranKeluar = false;
  stopFan();
  if (alasan != "") bot.sendMessage(CHAT_ID, "\u2601\ufe0f Ditutup: " + alasan, "");
}

void stopMotor() {
  ledcWrite(PWM_CHANNEL, 0);
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
}

void nyalakanFan() {
  digitalWrite(FAN_IN1, HIGH);
  digitalWrite(FAN_IN2, LOW);
}

void stopFan() {
  digitalWrite(FAN_IN1, LOW);
  digitalWrite(FAN_IN2, LOW);
}

void handleSensorLogic() {
  int ldr = analogRead(LDR_PIN);
  int rain = digitalRead(RAIN_PIN);
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (debugMode) {
    String dbg = "[DBG] LDR=" + String(ldr) + ", Rain=" + (rain == LOW ? "YA" : "TIDAK") +
                 ", Temp=" + String(temp) + ", Hum=" + String(hum);
    Serial.println(dbg);
  }

  if (!modeOtomatis) return;

  if (isnan(hum)) {
    stopFan(); // fallback safety
    return;
  }

  if (hum > HUM_THRESHOLD) nyalakanFan();
  else stopFan();

  if (ldr >= LDR_THRESHOLD && jemuranKeluar) tutupJemuran("Gelap");
  if (rain == LOW && jemuranKeluar) tutupJemuran("Hujan");
  if (ldr < LDR_THRESHOLD && rain != LOW && !jemuranKeluar) bukaJemuran("Cerah");
}


void sendStatus(String chat_id) {
  int ldr = analogRead(LDR_PIN);
  int rain = digitalRead(RAIN_PIN);
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  String cahaya = ldr < LDR_THRESHOLD ? "Terang" : "Gelap";
  String hujan = rain == LOW ? "Hujan" : "Tidak";
  String fanStatus = digitalRead(FAN_IN1) == HIGH ? "ON" : "OFF";
  String jemuranStatus = jemuranKeluar ? "Keluar" : "Masuk";
  String mode = modeOtomatis ? "Otomatis" : "Manual";

  String msg = "\u2139 STATUS:\n";
  msg += isnan(temp) ? "Suhu: ERROR\n" : "Suhu: " + String(temp, 1) + " \u00b0C\n";
  msg += isnan(hum)  ? "Lembab: ERROR\n" : "Lembab: " + String(hum, 1) + " %\n";
  msg += "Cahaya: " + cahaya + " (" + String(ldr) + ")\n";
  msg += "Hujan: " + hujan + "\n";
  msg += "Kipas: " + fanStatus + "\n";
  msg += "Jemuran: " + jemuranStatus + "\n";
  msg += "Mode: " + mode + "\n";
  msg += "WiFi IP: " + WiFi.localIP().toString();

  bot.sendMessage(chat_id, msg, "");
}

void parseMotorCommand(String text, int &speed, int &duration, bool &reverse) {
  speed = 150;
  duration = 1000;
  reverse = false;

  if (text.indexOf("rv") != -1) reverse = true;
  text.replace("/test_motor", "");
  text.replace("fw", "");
  text.replace("rv", "");
  text.trim();

  int sp = text.indexOf(' ');
  if (sp != -1) {
    speed = text.substring(0, sp).toInt();
    duration = text.substring(sp + 1).toInt();
  }

  speed = constrain(speed, 0, 255);
  duration = constrain(duration, 100, 10000);
}

void parseSetThreshold(String text) {
  if (text.startsWith("/set_temp")) {
    TEMP_THRESHOLD = text.substring(10).toFloat();
  } else if (text.startsWith("/set_hum")) {
    HUM_THRESHOLD = text.substring(9).toFloat();
  } else if (text.startsWith("/set_ldr")) {
    LDR_THRESHOLD = text.substring(9).toInt();
  }
}

void handleTelegramMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String text = bot.messages[i].text;
    String chat_id = bot.messages[i].chat_id;

    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "⛔ Akses ditolak.", "");
      continue;
    }

    if (text == "/start") {
      bot.sendMessage(chat_id,
        "Perintah:\n"
        "/buka, /tutup, /status\n"
        "/manual, /auto\n"
        "/test_motor [fw/rv] [speed] [ms]\n"
        "/test_fan\n"
        "/set_temp 29.5\n/set_hum 70\n/set_ldr 2400\n"
        "/debug_on, /debug_off\n"
        "/reset"
      );
    } else if (text == "/buka") {
      bukaJemuran("Manual");
    } else if (text == "/tutup") {
      tutupJemuran("Manual");
    } else if (text == "/manual") {
      modeOtomatis = false;
      bot.sendMessage(chat_id, "Mode: Manual");
    } else if (text == "/auto") {
      modeOtomatis = true;
      bot.sendMessage(chat_id, "Mode: Otomatis");
    } else if (text == "/status") {
      sendStatus(chat_id);
	} else if (text == "/debug_on") {
      debugMode = true;
      bot.sendMessage(chat_id, "🟢 Debug mode AKTIF.");
    } else if (text == "/debug_off") {
      debugMode = false;
      bot.sendMessage(chat_id, "🔴 Debug mode NONAKTIF.");
    } else if (text.startsWith("/set_")) {
      parseSetThreshold(text);
      bot.sendMessage(chat_id, "✅ Threshold diperbarui.");
    } else if (text.startsWith("/test_motor")) {
      int speed, dur;
      bool rev;
      parseMotorCommand(text, speed, dur, rev);
      digitalWrite(MOTOR_IN1, rev ? LOW : HIGH);
      digitalWrite(MOTOR_IN2, rev ? HIGH : LOW);
      ledcWrite(PWM_CHANNEL, speed);
      delay(dur);
      stopMotor();
      bot.sendMessage(chat_id, "✅ Motor selesai dijalankan.");
    } else if (text == "/test_fan") {
      bot.sendMessage(chat_id, "Menguji kipas selama 2 detik...");
      digitalWrite(FAN_IN1, HIGH);
      digitalWrite(FAN_IN2, LOW);
      delay(2000);
      stopFan();
      bot.sendMessage(chat_id, "Fan test selesai.");
    } else if (text == "/reset") {
      prefs.begin("flags", false);
      prefs.putBool("justReset", true);
      prefs.end();
      bot.sendMessage(chat_id, "🔁 Restarting...");
      delay(1000);
      ESP.restart();
    } else {
      bot.sendMessage(chat_id, "Perintah tidak dikenali.", "");
    }
  }
}
//v.1.2.3 fix2