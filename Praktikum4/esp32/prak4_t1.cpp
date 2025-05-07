#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ESP32Servo.h>

// WiFi & Telegram setup
const char* ssid = "alwi ganteng";
const char* password = "gatau gua";
const char* botToken = "xxx";

WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);

// Sensor & actuator
const int ldrPin = 35;
const int servoPin = 13;
Servo ventServo;

// Threshold & state
const int openThreshold = 2700;   // Terlalu terang → buka ventilasi
const int closeThreshold = 3350;  // Redup lagi → tutup ventilasi
bool ventOpen = false;
bool manualOverride = false;

// Subscribers
std::vector<String> subscribers;
unsigned long lastPush = 0;
const unsigned long interval = 10000;

// Setup
void setup() {
  Serial.begin(115200);
  ventServo.attach(servoPin);
  closeVent();

  connectToWiFi();
  client.setInsecure();
  Serial.println("System started");
}

void loop() {
  int light = analogRead(ldrPin);
  Serial.print("LDR: ");
  Serial.print(light);
  Serial.print(" | Vent: ");
  Serial.print(ventOpen ? "Open" : "Closed");
  Serial.print(" | Manual: ");
  Serial.println(manualOverride ? "Yes" : "No");

  if (!manualOverride) {
    if (light < openThreshold && !ventOpen) {
      openVent();
    } else if (light > closeThreshold && ventOpen) {
      closeVent();
    }
  }


  if (millis() - lastPush >= interval) {
    pushUpdate(light);
    lastPush = millis();
  }

  int numNew = bot.getUpdates(bot.last_message_received + 1);
  for (int i = 0; i < numNew; i++) {
    handleCommand(bot.messages[i].chat_id, bot.messages[i].text);
  }

  Serial.println("Getting Telegram updates...");
  Serial.println("New messages: " + String(numNew));


  delay(100); 
}

void connectToWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

void openVent() {
  if (!ventOpen) {
    ventServo.write(90);
    ventOpen = true;
    Serial.println("Vent opened");
  }
}

void closeVent() {
  if (ventOpen) {
    ventServo.write(0);
    ventOpen = false;
    Serial.println("Vent closed");
  }
}


void handleCommand(String chat_id, String msg) {
  msg.trim();

  if (msg == "/vent_open") {
    openVent();
    manualOverride = true;
    bot.sendMessage(chat_id, "\xf0\x9f\x94\x93 Vent opened (manual)", "");
  } else if (msg == "/vent_close") {
    closeVent();
    manualOverride = true;
    bot.sendMessage(chat_id, "\xf0\x9f\x94\x92 Vent closed (manual)", "");
  } else if (msg == "/auto") {
    manualOverride = false;
    bot.sendMessage(chat_id, "✅ Auto mode re-enabled", "");
  } else if (msg == "/subscribe") {
    if (std::find(subscribers.begin(), subscribers.end(), chat_id) == subscribers.end()) {
      subscribers.push_back(chat_id);
      bot.sendMessage(chat_id, "\xe2\x9c\x85 Subscribed", "");
    } else {
      bot.sendMessage(chat_id, "\xe2\x9a\xa0 Already subscribed", "");
    }
  } else if (msg == "/unsubscribe") {
    auto it = std::remove(subscribers.begin(), subscribers.end(), chat_id);
    if (it != subscribers.end()) {
      subscribers.erase(it, subscribers.end());
      bot.sendMessage(chat_id, "\xe2\x9d\x8e Unsubscribed", "");
    } else {
      bot.sendMessage(chat_id, "\xe2\x9a\xa0 You are not subscribed", "");
    }
  } else if (msg == "/status") {
    int light = analogRead(ldrPin);
    String status = "\xf0\x9f\x8c\x9e Light: " + String(light) +
                    "\nVent: " + (ventOpen ? "Open" : "Closed") +
                    "\nMode: " + (manualOverride ? "Manual" : "Auto");
    bot.sendMessage(chat_id, status, "");
  } else if (msg == "/help" || msg == "/start") {
    String help = "\xf0\x9f\x93\x98 Commands:\n"
                  "/status\n"
                  "/vent_open\n"
                  "/vent_close\n"
                  "/auto\n"
                  "/subscribe\n"
                  "/unsubscribe";
    bot.sendMessage(chat_id, help, "");
  } else {
    bot.sendMessage(chat_id, "Unknown command. Use /help", "");
  }

  Serial.println("Handling command: " + msg);
  Serial.println("From chat ID: " + chat_id);
}


void pushUpdate(int light) {
  if (subscribers.empty()) return;
  String msg = "\xe2\x8f\xb0 Auto Update:\n\xf0\x9f\x8c\x9e Light: " + String(light) +
               "\nVent: " + (ventOpen ? "Open" : "Closed");
  for (String chat_id : subscribers) {
    bot.sendMessage(chat_id, msg, "");
  }
}