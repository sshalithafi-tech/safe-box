/*
 * 04 - POTENTIOMETER TEST (with live servo control)
 * Safe Box — University of Oulu Prototyping Course
 *
 * Turn the potentiometer — the servo moves with it.
 * Also checks if dial is near the secret code position (512).
 * Hold it there to "unlock" — same logic as the final safe box code.
 *
 * No extra library needed.
 *
 * Wiring — Potentiometer:
 *   Left leg  → 3.3V (Pin 36)
 *   Right leg → GND  (Pin 38)
 *   Middle    → GP26 / A0 (Pin 31)
 *
 * Wiring — Servo:
 *   Signal → GP0  (Pin 1)
 *   VCC    → 3.3V (Pin 36)
 *   GND    → GND  (Pin 38)
 */

#include <Servo.h>

Servo myServo;

#define SERVO_PIN      0
#define POT_PIN        A0
#define LOCKED_ANGLE   20
#define UNLOCKED_ANGLE 180

// Secret dial code: potentiometer must be near this raw value
const int SECRET_CODE = 512;   // midpoint of the dial
const int TOLERANCE   = 60;    // how close is close enough
const int HOLD_NEEDED  = 6;     // hold there for 6 reads = unlock

int holdCounter = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  myServo.attach(SERVO_PIN);
  myServo.write(LOCKED_ANGLE);
  Serial.println("Pot test — turn dial to move servo");
  Serial.println("Hold near 512 to unlock");
}

void loop() {
  int raw   = analogRead(POT_PIN);
  int angle = map(raw, 0, 1023, LOCKED_ANGLE, UNLOCKED_ANGLE);

  myServo.write(angle);

  Serial.print("Raw: "); Serial.print(raw);
  Serial.print("  Angle: "); Serial.print(angle);

  if (abs(raw - SECRET_CODE) < TOLERANCE) {
    holdCounter++;
    Serial.print("  HOLD "); Serial.print(holdCounter);
    Serial.print("/"); Serial.print(HOLD_NEEDED);
    if (holdCounter >= HOLD_NEEDED) {
      Serial.println("\n>>> CODE CORRECT — UNLOCKED!");
      holdCounter = 0;
      delay(2000);
    }
  } else {
    holdCounter = 0;
    Serial.print("  not at code");
  }

  Serial.println();
  delay(500);
}