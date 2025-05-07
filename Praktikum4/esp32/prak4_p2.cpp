#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// WiFi credentials
const char* ssid = "alwi ganteng";
const char* password = "gatau gua";

// Telegram BOT Token
const char* botToken = "XXXXXXXXXXX";
const char* chatId = "XXXXXXXXXXX"; // Chat ID untuk kirim notifikasi

WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);

// Sensor and actuator pins
const int smokeSensorPin = 35;  // Analog pin untuk sensor gas MQ-2
const int flameSensorPin = 34;  // Analog pin untuk sensor api
const int redLEDPin = 13;       // LED merah indikator bahaya
const int greenLEDPin = 12;     // LED hijau indikator aman

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Sensor thresholds
const int smokeDangerThreshold = 400;   // PPM ambang asap bahaya
const int flameDangerThreshold = 2000;  // Nilai ADC ambang api

// State variables
bool alertSent = false;
unsigned long lastAlertTime = 0;
const unsigned long alertThrottleTime = 10000;  // 10 detik antar notifikasi

// Sensor readings
int smokePPM = 0;
bool flameDetected = false;

void setup() {
  Serial.begin(115200);
  Serial.println("Fire Detection System Starting...");

  pinMode(smokeSensorPin, INPUT);
  pinMode(flameSensorPin, INPUT);
  pinMode(redLEDPin, OUTPUT);
  pinMode(greenLEDPin, OUTPUT);
  digitalWrite(redLEDPin, LOW);
  digitalWrite(greenLEDPin, LOW);

  lcd.init();
  lcd.backlight();

  connectToWiFi();
  client.setInsecure();
}

void loop() {
  readSensors();
  updateLcd();

  if (isDangerDetected()) {
    handleDangerState();
  } else {
    handleSafeState();
  }

  int numMessages = bot.getUpdates(bot.last_message_received);
  for (int i = 0; i < numMessages; i++) {
      processTelegramCommand(bot.messages[i].chat_id, bot.messages[i].text);
  }

  delay(1000); // delay 1 detik per loop
}

void connectToWiFi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  int retryCount = 0;

  while (WiFi.status() != WL_CONNECTED && retryCount < 20) {
    delay(500);
    Serial.print(".");
    retryCount++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to WiFi");
  }
}

void readSensors() {
  int smokeRaw = analogRead(smokeSensorPin);
  int flameRaw = analogRead(flameSensorPin);

  smokePPM = map(smokeRaw, 0, 4095, 0, 1000); // Konversi 0-4095 ADC ke 0-1000 PPM
  flameDetected = (flameRaw < flameDangerThreshold);

  Serial.print("Smoke PPM: ");
  Serial.print(smokePPM);
  Serial.print(" | Flame: ");
  Serial.println(flameDetected ? "DETECTED" : "Safe");
}

bool isDangerDetected() {
  return (smokePPM > smokeDangerThreshold || flameDetected);
}

void handleDangerState() {
  digitalWrite(redLEDPin, HIGH);
  digitalWrite(greenLEDPin, LOW);

  if (!alertSent || (millis() - lastAlertTime >= alertThrottleTime)) {
    String alertMessage = "⚠ ALERT: ";

    if (smokePPM > smokeDangerThreshold) {
      alertMessage += "Smoke detected (" + String(smokePPM) + " PPM)! ";
    }

    if (flameDetected) {
      alertMessage += "🔥 Fire detected!";
    }

    sendTelegramMessage(alertMessage);
    lastAlertTime = millis();
    alertSent = true;
  }
}

void handleSafeState() {
  digitalWrite(redLEDPin, LOW);
  digitalWrite(greenLEDPin, HIGH);

  if (alertSent) {
    sendTelegramMessage("✅ All clear! Smoke and fire no longer detected.");
    alertSent = false;
  }
}

void updateLcd() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Smoke: ");
  lcd.print(smokePPM);
  lcd.print(" PPM");

  lcd.setCursor(0, 1);
  lcd.print("Fire: ");
  lcd.print(flameDetected ? "DETECTED!" : "Safe");
}

void sendTelegramMessage(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    bot.sendMessage(chatId, message, "");
  }
}

void processTelegramCommand(String chat_id, String message) {
  if (message == "/start" || message == "/help") {
    String helpMessage = "Fire Detection System Commands:\n";
    helpMessage += "/status - Get current sensor readings\n";
    helpMessage += "/reset - Reset system alerts\n";
    bot.sendMessage(chat_id, helpMessage, "");
  }
  else if (message == "/status") {
    String statusMessage = "📊 Current Status:\n";
    statusMessage += "Smoke Level: " + String(smokePPM) + " PPM\n";
    statusMessage += "Fire Detected: " + String(flameDetected ? "YES ⚠" : "No ✅") + "\n";
    bot.sendMessage(chat_id, statusMessage, "");
  }
  else if (message == "/reset") {
    alertSent = false;
    bot.sendMessage(chat_id, "System alerts have been reset.", "");
  }
  else {
    bot.sendMessage(chat_id, "Unknown command. Send /help for available commands.", "");
  }
}