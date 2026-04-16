#include <Wire.h>
#include <MPU6050.h>
#include <Normal_vs_Seizure_inferencing.h>

MPU6050 mpu;
bool seizure_detected = false;
#define SAMPLE_RATE 50
#define SAMPLE_INTERVAL_MS (1000 / SAMPLE_RATE)
#define AXES 6

float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];

unsigned long last_sample_time = 0;
int feature_ix = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin(8, 9);

  mpu.initialize();

  if (!mpu.testConnection()) {
    Serial.println("MPU6050 error");
    while (1);
  }

  Serial.println("System ready");
}

void loop() {

  // Maintain exact sampling rate
  if (millis() - last_sample_time < SAMPLE_INTERVAL_MS) return;
  last_sample_time = millis();

  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // Add new sample
  features[feature_ix++] = ax / 16384.0;
  features[feature_ix++] = ay / 16384.0;
  features[feature_ix++] = az / 16384.0;
  features[feature_ix++] = gx / 131.0;
  features[feature_ix++] = gy / 131.0;
  features[feature_ix++] = gz / 131.0;

  // When buffer is full → run inference
  if (feature_ix >= EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {

    signal_t signal;
    numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);

    ei_impulse_result_t result = { 0 };
    run_classifier(&signal, &result, false);

    float seizure = result.classification[1].value;
    float normal  = result.classification[0].value;

    Serial.print("Seizure: ");
    Serial.print(seizure, 3);
    Serial.print(" | Normal: ");
    Serial.println(normal, 3);

    // Simple detection logic
    if (seizure > 0.8) {
      Serial.println("⚠️ SEIZURE DETECTED");
    }

    // 🔁 Sliding window: shift buffer (keep overlap)
    memmove(features, features + AXES, 
            (EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE - AXES) * sizeof(float));

    feature_ix = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE - AXES;
  }
}