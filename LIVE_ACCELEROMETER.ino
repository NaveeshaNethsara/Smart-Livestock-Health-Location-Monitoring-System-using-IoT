#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>

// ================= WIFI =================
#define WIFI_SSID "Naveesha's A56"
#define WIFI_PASSWORD "nb2k6900"

// ================= FIREBASE =================
#define API_KEY "AIzaSyAu7nrubc0mqtKsmA22mgLLpQkYAkAEy74"
#define DATABASE_URL "https://livestock-monitoring-sys-ad970-default-rtdb.asia-southeast1.firebasedatabase.app/"

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// MPU6050 I2C address
#define MPU_ADDR 0x68

void setup() {

  Serial.begin(115200);

  // I2C setup
  Wire.begin(21, 22);
  Wire.setClock(400000);

  // Wake up MPU6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  // ================= WIFI =================
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nWiFi Connected!");

  // ================= FIREBASE CONFIG =================
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // 🔥 Anonymous authentication
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase SignUp OK");
  } else {
    Serial.printf("SignUp failed: %s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("Firebase Ready!");
}

void loop() {

  int16_t ax, ay, az;

  // Read Accelerometer
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 6, true);

  ax = Wire.read() << 8 | Wire.read();
  ay = Wire.read() << 8 | Wire.read();
  az = Wire.read() << 8 | Wire.read();

  Serial.println("Sending Data to Firebase...");

  if (Firebase.ready()) {

    if (Firebase.RTDB.setInt(&fbdo, "/livestock/accelerometer/x", ax))
      Serial.println("X Sent OK");
    else
      Serial.println(fbdo.errorReason());

    if (Firebase.RTDB.setInt(&fbdo, "/livestock/accelerometer/y", ay))
      Serial.println("Y Sent OK");
    else
      Serial.println(fbdo.errorReason());

    if (Firebase.RTDB.setInt(&fbdo, "/livestock/accelerometer/z", az))
      Serial.println("Z Sent OK");
    else
      Serial.println(fbdo.errorReason());
  }

  Serial.println("---------------------------");
  delay(2000);
}