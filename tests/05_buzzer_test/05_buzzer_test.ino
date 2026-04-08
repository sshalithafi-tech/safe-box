/*
 * 05 - ACTIVE BUZZER TEST
 * Safe Box — University of Oulu Prototyping Course
 *
 * Plays three patterns in sequence:
 *   1. Startup beep  (2 short beeps = system ready)
 *   2. Unlock beep   (1 long beep = box opened)
 *   3. Tamper alarm  (5 rapid bursts = tamper alert)
 *
 * Note: this is for an ACTIVE buzzer.
 * If nothing sounds, check polarity (+ to GP15, - to GND).
 *
 * No extra library needed.
 *
 * Wiring:
 *   Buzzer + (long leg)  → GP15 (Physical Pin 20)
 *   Buzzer − (short leg) → GND  (Physical Pin 38)
 */

#define BUZZER_PIN 15

// Single beep helper — ms = how long the beep lasts
void beep(int ms) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(ms);
  digitalWrite(BUZZER_PIN, LOW);
}

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // Pattern 1: Startup — 2 short beeps
  Serial.println("Pattern 1: Startup beep");
  beep(120); delay(150);
  beep(120);
  delay(1500);

  // Pattern 2: Unlocked — 1 long beep
  Serial.println("Pattern 2: Unlocked beep");
  beep(600);
  delay(1500);

  // Pattern 3: Tamper alarm — 3 bursts of 5 rapid beeps
  Serial.println("Pattern 3: Tamper alarm");
  for (int burst = 0; burst < 3; burst++) {
    for (int i = 0; i < 5; i++) {
      beep(80); delay(60);
    }
    delay(350);
  }
  Serial.println("All patterns done. Starting loop...");
}

void loop() {
  // Repeat tamper alarm every 5 seconds to keep testing
  delay(5000);
  Serial.println("Tamper alarm again...");
  for (int burst = 0; burst < 3; burst++) {
    for (int i = 0; i < 5; i++) {
      beep(80); delay(60);
    }
    delay(350);
  }
}