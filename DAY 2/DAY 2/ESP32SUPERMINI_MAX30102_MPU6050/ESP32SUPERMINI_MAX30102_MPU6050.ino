#include <Wire.h>
#include "MAX30105.h"
#include "MPU6050.h"

MAX30105 maxSensor;
MPU6050 mpu;

// ---------------- HEART RATE ----------------
long lastBeatTime = 0;
float bpm = 0;
float baselineBPM = 75;
unsigned long lastBaselineUpdate = 0;

// ---------------- MOTION ----------------
int16_t axRaw, ayRaw, azRaw;
float ax, ay, az;

float prevMag = 0;
unsigned long lastPeakTime = 0;
float motionFreq = 0;

// ---------------- SEIZURE ----------------
bool seizureActive = false;
unsigned long seizureStart = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // I2C pins (YOUR CONFIG)
  Wire.begin(4, 5);

  // MAX30102 init
  if (!maxSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 not found");
    while (1);
  }

  maxSensor.setup();
  maxSensor.setPulseAmplitudeRed(0x0A);
  maxSensor.setPulseAmplitudeGreen(0);

  // MPU6050 init
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 not found");
    while (1);
  }

  Serial.println("System Ready");
}

void loop() {

  // ================= HEART RATE =================
  long ir = maxSensor.getIR();
  static long prevIR = 0;

  if (ir > 50000 && prevIR < ir) {
    unsigned long now = millis();

    if (lastBeatTime > 0) {
      bpm = 60000.0 / (now - lastBeatTime);
    }

    lastBeatTime = now;
  }

  prevIR = ir;

  // Update baseline slowly (every 5 sec)
  if (millis() - lastBaselineUpdate > 5000) {
    baselineBPM = (baselineBPM * 0.8) + (bpm * 0.2);
    lastBaselineUpdate = millis();
  }

  // ================= MOTION =================
  mpu.getAcceleration(&axRaw, &ayRaw, &azRaw);

  // Convert to g-units
  ax = axRaw / 16384.0;
  ay = ayRaw / 16384.0;
  az = azRaw / 16384.0;

  float mag = sqrt(ax * ax + ay * ay + az * az);

  // Peak detection
  if (mag > 1.2 && prevMag < mag) {
    unsigned long now = millis();

    if (lastPeakTime > 0) {
      motionFreq = 1000.0 / (now - lastPeakTime); // Hz
    }

    lastPeakTime = now;
  }

  prevMag = mag;

  // ================= SEIZURE LOGIC =================
  bool highHR = (bpm > baselineBPM + 30) || (bpm > 120);
  bool rhythmicMotion = (motionFreq > 3 && motionFreq < 8);

  if (highHR && rhythmicMotion) {
    if (!seizureActive) {
      seizureStart = millis();
      seizureActive = true;
    }

    // Require 5 sec continuous condition
    if (millis() - seizureStart > 5000) {
      Serial.println("⚠️ SEIZURE DETECTED ⚠️");
    }
  } else {
    seizureActive = false;
  }

  // ================= DEBUG =================
  Serial.print("BPM: ");
  Serial.print(bpm);

  Serial.print(" | Base: ");
  Serial.print(baselineBPM);

  Serial.print(" | MotionHz: ");
  Serial.print(motionFreq);

  Serial.print(" | Alert: ");
  Serial.println(seizureActive ? "YES" : "NO");

  delay(20); // ~50Hz loop
}