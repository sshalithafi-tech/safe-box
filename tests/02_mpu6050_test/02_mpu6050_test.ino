/*
 * 03 - MPU6050 ACCELEROMETER TEST
 * Safe Box — University of Oulu Prototyping Course
 *
 * Reads X, Y, Z acceleration every 100ms.
 * Calculates movement delta and flags TAMPER if it is too high.
 * Replace TAMPER_THRESHOLD with your value from the calibration sketch.
 *
 * Libraries needed — add to libraries.txt in Wokwi:
 *   Adafruit MPU6050
 *   Adafruit Unified Sensor
 *
 * Wiring:
 *   VCC → 3.3V (Pin 36)
 *   GND → GND  (Pin 38)
 *   SDA → GP4  (Pin 6)
 *   SCL → GP5  (Pin 7)
 *   AD0 → GND  (Pin 38)  sets I2C address to 0x68
 */

#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;

// Replace with YOUR value from the calibration sketch
#define TAMPER_THRESHOLD  2.0

float prevX = 0, prevY = 0, prevZ = 9.8;
bool firstRead = true;

void setup() {
  Serial.begin(115200);
  delay(1500);

  Wire.setSDA(4);
  Wire.setSCL(5);
  Wire.begin();

  if (!mpu.begin()) {
    Serial.println("ERROR: MPU6050 not found. Check SDA/SCL wiring.");
    while (1) delay(500);
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Serial.println("MPU6050 ready. Monitoring for tamper...");
  Serial.print("Threshold: "); Serial.println(TAMPER_THRESHOLD);
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  if (firstRead) {
    prevX = a.acceleration.x;
    prevY = a.acceleration.y;
    prevZ = a.acceleration.z;
    firstRead = false;
    delay(100);
    return;
  }

  float delta = abs(a.acceleration.x - prevX)
              + abs(a.acceleration.y - prevY)
              + abs(a.acceleration.z - prevZ);

  prevX = a.acceleration.x;
  prevY = a.acceleration.y;
  prevZ = a.acceleration.z;

  Serial.print("Delta: "); Serial.print(delta, 2);

  if (delta > TAMPER_THRESHOLD) {
    Serial.println("  <<< TAMPER DETECTED! >>>");
  } else {
    Serial.println("  [still]");
  }

  delay(100);
}