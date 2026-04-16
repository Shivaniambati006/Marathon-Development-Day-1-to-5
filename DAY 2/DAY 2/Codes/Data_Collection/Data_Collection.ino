#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

// Sampling rate (Hz)
#define SAMPLE_RATE 50
#define SAMPLE_INTERVAL_MS (1000 / SAMPLE_RATE)

unsigned long lastSampleTime = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin(8, 9); // SDA, SCL for ESP32-C3

  Serial.println("Initializing MPU6050...");
  mpu.initialize();

  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed!");
    while (1);
  }

  Serial.println("MPU6050 connected.");

  // Edge Impulse CSV header
  Serial.println("timestamp,accX,accY,accZ,gyrX,gyrY,gyrZ");
}

void loop() {
  if (millis() - lastSampleTime >= SAMPLE_INTERVAL_MS) {
    lastSampleTime = millis();

    int16_t ax, ay, az;
    int16_t gx, gy, gz;

    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    // Convert raw values to standard units
    float accX = ax / 16384.0;  // g
    float accY = ay / 16384.0;
    float accZ = az / 16384.0;

    float gyrX = gx / 131.0;    // deg/s
    float gyrY = gy / 131.0;
    float gyrZ = gz / 131.0;

    // Print CSV line
    Serial.print(millis());
    Serial.print(",");
    Serial.print(accX, 4); Serial.print(",");
    Serial.print(accY, 4); Serial.print(",");
    Serial.print(accZ, 4); Serial.print(",");
    Serial.print(gyrX, 4); Serial.print(",");
    Serial.print(gyrY, 4); Serial.print(",");
    Serial.println(gyrZ, 4);
  }
}