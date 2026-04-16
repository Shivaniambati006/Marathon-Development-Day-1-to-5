#include <Wire.h>
#include <MPU6050.h>
#include <Normal_vs_Seizure_inferencing.h>

MPU6050 mpu;

#define SAMPLE_RATE 50
#define SAMPLE_INTERVAL_MS (1000 / SAMPLE_RATE)

float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];

void setup() {
  Serial.begin(115200);
  Wire.begin(8, 9);

  mpu.initialize();

  if (!mpu.testConnection()) {
    Serial.println("MPU6050 error");
    while (1);
  }
}

void loop() {
  for (int i = 0; i < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; i += 6) {
    
    int16_t ax, ay, az, gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    features[i + 0] = ax / 16384.0;
    features[i + 1] = ay / 16384.0;
    features[i + 2] = az / 16384.0;
    features[i + 3] = gx / 131.0;
    features[i + 4] = gy / 131.0;
    features[i + 5] = gz / 131.0;

    delay(SAMPLE_INTERVAL_MS);
  }

  signal_t signal;
  numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);

  ei_impulse_result_t result = { 0 };

  EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);

  if (res != EI_IMPULSE_OK) {
    Serial.println("Classifier error");
    return;
  }

  // Print results
  Serial.println("Predictions:");
  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    Serial.print(result.classification[ix].label);
    Serial.print(": ");
    Serial.println(result.classification[ix].value, 4);
  }

  Serial.println("-------------------");
}