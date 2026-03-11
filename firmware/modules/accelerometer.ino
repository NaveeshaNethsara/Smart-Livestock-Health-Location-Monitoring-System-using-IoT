#include <Wire.h>

// MPU6050 I2C address
#define MPU_ADDR 0x68

void setup() {
  Wire.begin(21, 22);       // SDA = GPIO21, SCL = GPIO22
  Wire.setClock(400000);    // fast I2C for ESP32
  Serial.begin(115200);
  delay(1000);

  Serial.println("Initializing MPU6050...");

  // Wake up MPU6050 (it starts in sleep mode)
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0);    // set to 0 to wake up
  Wire.endTransmission(true);

  Serial.println("MPU6050 should now be awake!");
}

void loop() {
  int16_t ax, ay, az;
  

  // Read accelerometer and gyro data
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // starting register for ACCEL_XOUT_H
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 14, true); // 14 bytes: accel(6)+temp(2)+gyro(6)

  ax = Wire.read() << 8 | Wire.read();
  ay = Wire.read() << 8 | Wire.read();
  az = Wire.read() << 8 | Wire.read();

 

  Serial.println("----- Accelerometer -----");
  Serial.print("X: "); Serial.print(ax);
  Serial.print("  Y: "); Serial.print(ay);
  Serial.print("  Z: "); Serial.println(az);


  

  Serial.println();
  delay(1000);
}