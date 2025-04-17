#define LDR_PIN      32   // Pin sensor LDR (ADC)
#define LED_PIN      4    // Pin LED
#define BUTTON_PIN   15   // Pin tombol override

bool lampuMenyala  = false;  // Status lampu (mulai mati)
bool tombolDitekan = false;  // Flag debouncing tombol

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);   // Tombol dengan internal pull‑up

  digitalWrite(LED_PIN, LOW);          // Pastikan LED mati saat startup
}

void loop() {
  int nilaiLDR     = analogRead(LDR_PIN);     // Baca sensor LDR
  int statusTombol = digitalRead(BUTTON_PIN); // HIGH = tidak ditekan, LOW = ditekan

  Serial.print("Nilai LDR: ");
  Serial.print(nilaiLDR);
  Serial.print(" | Status Tombol: ");
  Serial.println(statusTombol);

  // ── Logika sensor ───────────────────────────────────────────────
  // Jika gelap (nilai < 3000) dan lampu mati ➜ nyalakan
  if (nilaiLDR < 3000 && !lampuMenyala) {
    digitalWrite(LED_PIN, HIGH);
    lampuMenyala = true;
    Serial.println("Lampu menyala otomatis (gelap).");
  }

  // Jika terang (nilai ≥ 3000) dan lampu menyala ➜ matikan
  else if (nilaiLDR >= 3000 && lampuMenyala) {
    digitalWrite(LED_PIN, LOW);
    lampuMenyala = false;
    Serial.println("Lampu mati otomatis (terang).");
  }

  // ── Logika tombol override ──────────────────────────────────────
  if (statusTombol == LOW && !tombolDitekan) {
    // Toggle lampu
    lampuMenyala = !lampuMenyala;
    digitalWrite(LED_PIN, lampuMenyala ? HIGH : LOW);
    tombolDitekan = true;              // Hindari bouncing

    Serial.println(lampuMenyala
      ? "Lampu dinyalakan oleh tombol."
      : "Lampu dimatikan oleh tombol.");

    delay(3000);                       // Debounce panjang
  }

  // Reset flag ketika tombol dilepas
  if (statusTombol == HIGH) {
    tombolDitekan = false;
  }

  delay(200);                          // Jeda antar pembacaan
}
