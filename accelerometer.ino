#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

float ax, ay, az;
float magnitude;
float threshold = 1.5;   // Adjust based on testing

void setup() {
  Serial.begin(115200);
  Wire.begin();

  Serial.println("Initializing MPU6050...");

  mpu.initialize();

  if (mpu.testConnection()) {
    Serial.println("MPU6050 Connected Successfully!");
  } else {
    Serial.println("MPU6050 Connection Failed!");
    while (1);
  }
}

void loop() {

  int16_t rawAx, rawAy, rawAz;

  mpu.getAcceleration(&rawAx, &rawAy, &rawAz);

  // Convert raw values to g-force
  ax = rawAx / 16384.0;
  ay = rawAy / 16384.0;
  az = rawAz / 16384.0;

  // Calculate total movement magnitude
  magnitude = sqrt(ax * ax + ay * ay + az * az);

  Serial.println("------ Accelerometer Data ------");
  Serial.print("X: "); Serial.print(ax);
  Serial.print("  Y: "); Serial.print(ay);
  Serial.print("  Z: "); Serial.println(az);
  Serial.print("Total Movement: ");
  Serial.println(magnitude);

  // Activity detection
  if (magnitude > threshold) {
    Serial.println("⚠️ Animal Movement Detected!");
  } else {
    Serial.println("Animal is Calm");
  }

  Serial.println("--------------------------------\n");

  delay(1000);
}