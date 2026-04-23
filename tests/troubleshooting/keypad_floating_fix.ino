const int buttonPin = 2;

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  Serial.begin(115200);
}

void loop() {
  if (digitalRead(buttonPin) == LOW) {
    Serial.println("Pressed");
  } else {
    Serial.println("Released");
  }
  delay(200);
}