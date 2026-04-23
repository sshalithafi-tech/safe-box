/*
 * SAFE BOX — FINAL CODE
 * University of Oulu Prototyping Course
 *
 * Keys:
 *   0-9     → enter PIN digits
 *   #       → confirm PIN
 *   *       → clear PIN entry
 *   A,B,C,D → silence tamper alarm
 *
 * Servo angles:  LOCKED = 20 deg  |  UNLOCKED = 180 deg
 *
 * WIRING:
 *   Servo signal  → GP0  (Pin 1)   | VCC → 3.3V (Pin 36) | GND → Pin 38
 *   Keypad R1-R4  → GP2-GP5        | C1-C4 → GP6-GP9
 *   MPU6050 VIN   → 3.3V (Pin 36)  | GND → Pin 38
 *   MPU6050 SDA   → GP16 (Pin 21)  | SCL → GP17 (Pin 22)
 *   Passive buzz  → GP15 (Pin 20)  | GND → Pin 38
 *   Red LED       → 330Ω → GP13 (Pin 17) | GND → Pin 38
 *   Green LED     → 330Ω → GP14 (Pin 19) | GND → Pin 38
 *   Battery +     → VSYS (Pin 39)  | Battery - → GND (Pin 38)
 */

#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Servo.h>

// ── Pin definitions ───────────────────────────────────────────────────────
const int SERVO_PIN   = 0;
const int RED_LED     = 13;
const int GREEN_LED   = 14;
const int BUZZER_PIN  = 15;
const int SDA_PIN     = 16;
const int SCL_PIN     = 17;

// ── Servo angles ──────────────────────────────────────────────────────────
const int LOCKED_ANGLE   = 20;
const int UNLOCKED_ANGLE = 180;
const int SWEEP_DELAY    = 12;

// ── Buzzer frequency ──────────────────────────────────────────────────────
const int BEEP_FREQ = 2000;

// ── Tamper calibration settings ───────────────────────────────────────────
const int   CALIBRATION_SAMPLES = 50;
const float CALIBRATION_BUFFER  = 5;

// ── Secret PIN — change to your own ──────────────────────────────────────
const String CORRECT_PIN = "5555";

// ── Keypad pins and layout ────────────────────────────────────────────────
const int rowPins[4] = {2, 3, 4, 5};
const int colPins[4] = {6, 7, 8, 9};

const char keyMap[4][4] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// ── Global objects ────────────────────────────────────────────────────────
Servo lockServo;
Adafruit_MPU6050 mpu;

// ── Global state ──────────────────────────────────────────────────────────
bool   isLocked        = true;
bool   alarmActive     = false;
bool   mpuFound        = false;
int    currentAngle    = LOCKED_ANGLE;
char   lastKey         = 0;
String enteredPIN      = "";

unsigned long lastPressTime = 0;
unsigned long lastAlarmBeep = 0;

float prevX          = 0.0;
float prevY          = 0.0;
float prevZ          = 0.0;
float tamperThreshold = 3.5;

// ═════════════════════════════════════════════════════════════════════════
//  FUNCTION DECLARATIONS (forward declarations to avoid order errors)
// ═════════════════════════════════════════════════════════════════════════
void beep(int ms);
void slowMove(int fromAngle, int toAngle);
void lockBox();
void unlockBox();
void silenceAlarm();
bool isSilenceKey(char key);
char scanKeypad();
void calibrateMPU();
void checkTamper();
void handleAlarm();

// ═════════════════════════════════════════════════════════════════════════
//  BEEP
// ═════════════════════════════════════════════════════════════════════════
void beep(int ms) {
  tone(BUZZER_PIN, BEEP_FREQ);
  delay(ms);
  noTone(BUZZER_PIN);
}

// ═════════════════════════════════════════════════════════════════════════
//  SERVO SWEEP
// ═════════════════════════════════════════════════════════════════════════
void slowMove(int fromAngle, int toAngle) {
  if (fromAngle <= toAngle) {
    for (int pos = fromAngle; pos <= toAngle; pos++) {
      lockServo.write(pos);
      delay(SWEEP_DELAY);
    }
  } else {
    for (int pos = fromAngle; pos >= toAngle; pos--) {
      lockServo.write(pos);
      delay(SWEEP_DELAY);
    }
  }
  currentAngle = toAngle;
}

// ═════════════════════════════════════════════════════════════════════════
//  LOCK / UNLOCK
// ═════════════════════════════════════════════════════════════════════════
void lockBox() {
  slowMove(currentAngle, LOCKED_ANGLE);
  isLocked = true;
  digitalWrite(RED_LED,   HIGH);
  digitalWrite(GREEN_LED, LOW);
  beep(100); delay(100); beep(100);
  Serial.println("LOCKED. Enter PIN:");
  Serial.println("A/B/C/D = silence alarm");
}

void unlockBox() {
  alarmActive = false;
  noTone(BUZZER_PIN);
  isLocked = false;
  digitalWrite(RED_LED,   LOW);
  digitalWrite(GREEN_LED, HIGH);
  slowMove(currentAngle, UNLOCKED_ANGLE);
  beep(500);
  Serial.println("UNLOCKED! Re-locking in 5 seconds...");
  delay(5000);
  lockBox();
}

// ═════════════════════════════════════════════════════════════════════════
//  ALARM SILENCE
// ═════════════════════════════════════════════════════════════════════════
bool isSilenceKey(char key) {
  return (key == 'A' || key == 'B' || key == 'C' || key == 'D');
}

void silenceAlarm() {
  alarmActive = false;
  noTone(BUZZER_PIN);
  enteredPIN  = "";
  Serial.println("Alarm silenced. Box remains LOCKED.");
  Serial.println("Enter PIN to unlock.");
  tone(BUZZER_PIN, 800, 200);
  delay(300);
}

// ═════════════════════════════════════════════════════════════════════════
//  KEYPAD SCAN
// ═════════════════════════════════════════════════════════════════════════
char scanKeypad() {
  for (int row = 0; row < 4; row++) {
    digitalWrite(rowPins[row], LOW);
    delayMicroseconds(10);
    for (int col = 0; col < 4; col++) {
      if (digitalRead(colPins[col]) == LOW) {
        digitalWrite(rowPins[row], HIGH);
        return keyMap[row][col];
      }
    }
    digitalWrite(rowPins[row], HIGH);
  }
  return 0;
}

// ═════════════════════════════════════════════════════════════════════════
//  MPU6050 CALIBRATION
// ═════════════════════════════════════════════════════════════════════════
void calibrateMPU() {
  Serial.println("Calibrating — keep the box STILL for 3 seconds...");

  for (int i = 0; i < 3; i++) {
    digitalWrite(RED_LED, HIGH); delay(200);
    digitalWrite(RED_LED, LOW);  delay(200);
  }
  digitalWrite(RED_LED, HIGH);

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float lx = a.acceleration.x;
  float ly = a.acceleration.y;
  float lz = a.acceleration.z;
  float maxDelta = 0.0;

  for (int i = 0; i < CALIBRATION_SAMPLES; i++) {
    mpu.getEvent(&a, &g, &temp);
    float delta = abs(a.acceleration.x - lx)
                + abs(a.acceleration.y - ly)
                + abs(a.acceleration.z - lz);
    if (delta > maxDelta) maxDelta = delta;
    lx = a.acceleration.x;
    ly = a.acceleration.y;
    lz = a.acceleration.z;
    delay(50);
  }

  tamperThreshold = maxDelta + CALIBRATION_BUFFER;
  prevX = lx;
  prevY = ly;
  prevZ = lz;

  Serial.print("Calibration done. Threshold set to: ");
  Serial.print(tamperThreshold, 2);
  Serial.println(" m/s2");

  beep(80); delay(80);
  beep(80); delay(80);
  beep(80);
}

// ═════════════════════════════════════════════════════════════════════════
//  TAMPER CHECK
// ═════════════════════════════════════════════════════════════════════════
void checkTamper() {
  if (!mpuFound || !isLocked || alarmActive) return;

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float delta = abs(a.acceleration.x - prevX)
              + abs(a.acceleration.y - prevY)
              + abs(a.acceleration.z - prevZ);

  prevX = a.acceleration.x;
  prevY = a.acceleration.y;
  prevZ = a.acceleration.z;

  if (delta > tamperThreshold) {
    Serial.print("TAMPER DETECTED! Delta: ");
    Serial.print(delta, 2);
    Serial.println(" m/s2");
    Serial.println("Press A/B/C/D to silence. Or enter correct PIN.");
    alarmActive = true;

    for (int i = 0; i < 4; i++) {
      digitalWrite(RED_LED, LOW);  delay(80);
      digitalWrite(RED_LED, HIGH); delay(80);
    }
  }
}

// ═════════════════════════════════════════════════════════════════════════
//  CONTINUOUS ALARM (non-blocking)
// ═════════════════════════════════════════════════════════════════════════
void handleAlarm() {
  if (!alarmActive) return;
  unsigned long now = millis();
  if (now - lastAlarmBeep > 300) {
    lastAlarmBeep = now;
    tone(BUZZER_PIN, BEEP_FREQ, 150);
  }
}

// ═════════════════════════════════════════════════════════════════════════
//  SETUP
// ═════════════════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("=== SAFE BOX STARTING ===");

  pinMode(RED_LED,   OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  digitalWrite(RED_LED,   LOW);
  digitalWrite(GREEN_LED, LOW);

  for (int r = 0; r < 4; r++) {
    pinMode(rowPins[r], OUTPUT);
    digitalWrite(rowPins[r], HIGH);
  }
  for (int c = 0; c < 4; c++) {
    pinMode(colPins[c], INPUT_PULLUP);
  }

  lockServo.attach(SERVO_PIN);
  lockServo.write(LOCKED_ANGLE);
  currentAngle = LOCKED_ANGLE;
  delay(500);

  Wire.setSDA(SDA_PIN);
  Wire.setSCL(SCL_PIN);
  Wire.begin();

  if (!mpu.begin()) {
    Serial.println("WARNING: MPU6050 not found — tamper detection disabled.");
    mpuFound = false;
  } else {
    Serial.println("MPU6050 found.");
    mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    mpuFound = true;
    calibrateMPU();
  }

  lockBox();
  Serial.println("# = confirm PIN  |  * = clear  |  A/B/C/D = silence alarm");
}

// ═════════════════════════════════════════════════════════════════════════
//  MAIN LOOP
// ═════════════════════════════════════════════════════════════════════════
void loop() {

  checkTamper();
  handleAlarm();

  char key = scanKeypad();

  if (key != 0) {
    if (key != lastKey || (millis() - lastPressTime) > 300) {
      lastKey       = key;
      lastPressTime = millis();

      // ── A, B, C, D → silence alarm ──────────────────────────────────
      if (isSilenceKey(key)) {
        if (alarmActive) {
          silenceAlarm();
        } else {
          Serial.print("Key ");
          Serial.print(key);
          Serial.println(": no alarm active.");
        }

      // ── * → clear PIN ────────────────────────────────────────────────
      } else if (key == '*') {
        enteredPIN = "";
        if (!alarmActive) beep(60);
        Serial.println("Cleared.");

      // ── # → confirm PIN ──────────────────────────────────────────────
      } else if (key == '#') {
        if (enteredPIN == CORRECT_PIN) {
          Serial.println("Correct PIN!");
          enteredPIN = "";
          unlockBox();
        } else {
          Serial.println("Wrong PIN. Try again.");
          enteredPIN = "";
          if (!alarmActive) {
            beep(80); delay(80);
            beep(80); delay(80);
            beep(80);
          }
        }

      // ── 0–9 → digit entry ────────────────────────────────────────────
      } else {
        enteredPIN += key;
        if (!alarmActive) beep(40);

        String stars = "";
        for (int i = 0; i < (int)enteredPIN.length(); i++) {
          stars += "*";
        }
        Serial.print("PIN: ");
        Serial.println(stars);

        if (enteredPIN.length() > 6) {
          enteredPIN = "";
          Serial.println("Too long — cleared.");
        }
      }
    }

  } else {
    lastKey = 0;
  }

  delay(20);
}