#include <Wire.h>
#include <math.h>

#define MPU_ADDR 0x68

// Detection parameters
#define MAG_THRESHOLD 2.5      // g
#define SEIZURE_TIME  15000    // ms (15 sec)

unsigned long seizureStart = 0;
bool seizureActive = false;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);  // SDA, SCL

  // Wake MPU-6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();

  Serial.println("MPU-6050 Ready");
}

void loop() {
  // Read 14 bytes
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 14, true);

  int16_t ax = (Wire.read() << 8) | Wire.read();
  int16_t ay = (Wire.read() << 8) | Wire.read();
  int16_t az = (Wire.read() << 8) | Wire.read();

  Wire.read(); Wire.read(); // temp ignore

  int16_t gx = (Wire.read() << 8) | Wire.read();
  int16_t gy = (Wire.read() << 8) | Wire.read();
  int16_t gz = (Wire.read() << 8) | Wire.read();

  // Convert to g
  float axg = ax / 16384.0;
  float ayg = ay / 16384.0;
  float azg = az / 16384.0;

  float mag = sqrt(axg*axg + ayg*ayg + azg*azg);

  Serial.print("MAG: ");
  Serial.print(mag);

  // Seizure detection
  if (mag > MAG_THRESHOLD) {
    if (!seizureActive) {
      seizureActive = true;
      seizureStart = millis();
    } else if (millis() - seizureStart > SEIZURE_TIME) {
      Serial.println("  >>> SEIZURE DETECTED <<<");
    }
  } else {
    seizureActive = false;
    Serial.println();
  }

  delay(100);
}