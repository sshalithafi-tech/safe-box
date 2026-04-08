/*
 * 02 - 4x4 KEYPAD TEST
 * Safe Box — University of Oulu Prototyping Course
 *
 * Reads key presses. Enter the correct PIN and press #.
 * Press * to clear and start again.
 *
 * Library: Keypad by Mark Stanley and Alexander Brevig
 * GitHub:  github.com/Chris--A/Keypad
 * Install: add "Keypad" to libraries.txt in Wokwi,
 *          or Tools > Manage Libraries > search "Keypad" in Arduino IDE.
 *
 * Wiring:
 *   R1 → GP2  (Pin 4)    C1 → GP8  (Pin 11)
 *   R2 → GP3  (Pin 5)    C2 → GP9  (Pin 12)
 *   R3 → GP6  (Pin 9)    C3 → GP10 (Pin 14)
 *   R4 → GP7  (Pin 10)   C4 → GP11 (Pin 15)
 */

#include <Keypad.h>

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {2, 3, 6, 7};
byte colPins[COLS] = {8, 9, 10, 11};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

const String CORRECT_PIN = "1234";  // change this to your PIN
String enteredPIN = "";

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Keypad ready. Enter PIN then press #");
  Serial.println("Press * to clear");
}

void loop() {
  char key = keypad.getKey();
  if (key == NO_KEY) return;

  Serial.print("Key: "); Serial.println(key);

  if (key == '*') {
    enteredPIN = "";
    Serial.println("Cleared.");

  } else if (key == '#') {
    Serial.print("Checking PIN: "); Serial.println(enteredPIN);
    if (enteredPIN == CORRECT_PIN) {
      Serial.println(">>> CORRECT — UNLOCKED!");
    } else {
      Serial.println("Wrong PIN. Try again.");
    }
    enteredPIN = "";

  } else {
    enteredPIN += key;
    if (enteredPIN.length() > 6) { enteredPIN = ""; Serial.println("Too long — cleared."); }
  }
}