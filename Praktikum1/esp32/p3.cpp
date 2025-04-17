#include <ESP32Servo.h>          // Library kompatibel ESP32

#define ECHO_PIN   4
#define TRIG_PIN   5
#define BUZZER_PIN 13
#define SERVO_PIN  14            // Dulu pakai GPIO 9, pindah ke GPIO 14

Servo myServo;

void setup() {
  myServo.attach(SERVO_PIN);     // Sambungkan servo
  pinMode(TRIG_PIN,   OUTPUT);
  pinMode(ECHO_PIN,   INPUT);
  pinMode(BUZZER_PIN, OUTPUT);   // Buzzer siap

  Serial.begin(115200);          // Baud tinggi biar responsif
  digitalWrite(TRIG_PIN, LOW);   // Reset sensor ultrasonik
}

void loop() {
  // Ping ultrasonik
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Hitung jarak
  long duration = pulseIn(ECHO_PIN, HIGH);
  int  distance = duration * 0.034 / 2;   // cm

  Serial.print("Jarak: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance > 0 && distance < 20) {    // Target terdeteksi < 20 cm
    digitalWrite(BUZZER_PIN, HIGH);       // Bunyikan buzzer sebentar
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);

    myServo.write(90);                    // Buka pintu (90°)
    delay(2000);                          // Tahan 2 detik
  } else {
    myServo.write(0);                     // Tutup pintu (0°)
  }

  delay(500);                             // Jeda untuk kestabilan
}
