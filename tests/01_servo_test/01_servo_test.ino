/*
 * 01 - SERVO MOTOR TEST
 * Safe Box — University of Oulu Prototyping Course
 *
 * Sweeps slowly from LOCKED (20°) to UNLOCKED (180°) and back.
 * These angles were found using the potentiometer calibration sketch.
 *
 * Library: Servo.h — already included, no install needed.
 *
 * Wiring:
 *   Servo signal (orange) → GP0  (Physical Pin 1)
 *   Servo VCC    (red)    → 3.3V (Physical Pin 36)
 *   Servo GND    (brown)  → GND  (Physical Pin 38)
 */

#include <Servo.h>

Servo lockServo;

#define SERVO_PIN      0
#define LOCKED_ANGLE   20   // found during calibration
#define UNLOCKED_ANGLE 180  // found during calibration
#define SWEEP_DELAY    12   // ms per degree — slow and safe for 3D printed parts

// Moves servo one degree at a time to protect the 3D printed arm
void slowMove(int fromAngle, int toAngle) {
  Serial.print("Moving: "); Serial.print(fromAngle);
  Serial.print(" to "); Serial.println(toAngle);
  if (fromAngle <= toAngle) {
    for (int p = fromAngle; p <= toAngle; p++) { lockServo.write(p); delay(SWEEP_DELAY); }
  } else {
    for (int p = fromAngle; p >= toAngle; p--) { lockServo.write(p); delay(SWEEP_DELAY); }
  }
  Serial.println("Done.");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  lockServo.attach(SERVO_PIN);
  lockServo.write(LOCKED_ANGLE);  // start locked
  delay(500);
  Serial.println("Servo test — LOCKED at 20, UNLOCKED at 180");
}

void loop() {
  Serial.println("--- UNLOCKING ---");
  slowMove(LOCKED_ANGLE, UNLOCKED_ANGLE);
  delay(2000);

  Serial.println("--- LOCKING ---");
  slowMove(UNLOCKED_ANGLE, LOCKED_ANGLE);
  delay(2000);
}