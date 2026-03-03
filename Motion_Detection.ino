#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <math.h>

// ================= WIFI =================
#define WIFI_SSID "Naveesha's A56"
#define WIFI_PASSWORD "nb2k6900"

// ================= FIREBASE =================
#define API_KEY "AIzaSyAu7nrubc0mqtKsmA22mgLLpQkYAkAEy74"
#define DATABASE_URL "https://livestock-monitoring-sys-ad970-default-rtdb.asia-southeast1.firebasedatabase.app"

// ================= MPU6050 =================
#define MPU_ADDR 0x68

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// ================= GLOBAL VARIABLES =================
String motionState = "Initializing";

float gravityOffset = 16384;     // will auto-calibrate
float movementBuffer[5];         // smoothing buffer
int bufferIndex = 0;

unsigned long lastMovementTime = 0;
unsigned long fallCheckTime = 0;

bool possibleFall = false;

// ================= READ MPU FUNCTION =================
void readMPU(int16_t &ax, int16_t &ay, int16_t &az) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 6, true);

  ax = Wire.read() << 8 | Wire.read();
  ay = Wire.read() << 8 | Wire.read();
  az = Wire.read() << 8 | Wire.read();
}

// ================= SETUP =================
void setup() {

  Serial.begin(115200);

  // I2C
  Wire.begin(21, 22);
  Wire.setClock(400000);

  // Wake MPU6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  Serial.println("Calibrating Gravity... Keep sensor still");

  // ===== AUTO GRAVITY CALIBRATION =====
  float sum = 0;
  for (int i = 0; i < 100; i++) {
    int16_t ax, ay, az;
    readMPU(ax, ay, az);
    float mag = sqrt(ax*ax + ay*ay + az*az);
    sum += mag;
    delay(20);
  }
  gravityOffset = sum / 100.0;

  Serial.print("Calibrated Gravity: ");
  Serial.println(gravityOffset);

  // WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nWiFi Connected");

  // Firebase
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  Firebase.signUp(&config, &auth, "", "");
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("Firebase Ready");

  lastMovementTime = millis();
}

// ================= LOOP =================
void loop() {

  int16_t ax, ay, az;
  readMPU(ax, ay, az);

  // ===== Magnitude Calculation =====
  float magnitude = sqrt(ax*ax + ay*ay + az*az);

  // Remove gravity
  float movement = abs(magnitude - gravityOffset);

  // ===== Moving Average Filter =====
  movementBuffer[bufferIndex] = movement;
  bufferIndex = (bufferIndex + 1) % 5;

  float smoothMovement = 0;
  for (int i = 0; i < 5; i++) {
    smoothMovement += movementBuffer[i];
  }
  smoothMovement /= 5.0;

  Serial.print("Smooth Movement: ");
  Serial.println(smoothMovement);

  // ================= MOTION CLASSIFICATION =================

  // 🐄 Standing Still
  if (smoothMovement < 600) {
    motionState = "Standing Still";
  }

  // 🐄 Normal Walking
  else if (smoothMovement >= 600 && smoothMovement < 3000) {
    motionState = "Normal Walking";
    lastMovementTime = millis();
  }

  // 🐄 Running
  else if (smoothMovement >= 3000 && smoothMovement < 9000) {
    motionState = "Running";
    lastMovementTime = millis();
  }

  // 🐄 Possible Fall (Large Spike)
  else if (smoothMovement >= 9000) {
    possibleFall = true;
    fallCheckTime = millis();
  }

  // ===== Fall Confirmation =====
  if (possibleFall && millis() - fallCheckTime > 500) {

    if (smoothMovement < 400) {
      motionState = "FALL DETECTED";
    } else {
      motionState = "Sudden Movement";
    }

    possibleFall = false;
  }

  // 🐄 Lying Down (Orientation Based)
  if (abs(az) < gravityOffset * 0.6 && smoothMovement < 600) {
    motionState = "Lying Down";
  }

  // 🐄 Long Inactivity (e.g., 30 minutes)
  if (millis() - lastMovementTime > 1800000) {
    motionState = "Long Inactivity Alert";
  }

  Serial.print("Detected Motion: ");
  Serial.println(motionState);

  // ================= FIREBASE UPDATE =================
  if (Firebase.ready()) {

    Firebase.RTDB.setInt(&fbdo, "/livestock/accelerometer/x", ax);
    Firebase.RTDB.setInt(&fbdo, "/livestock/accelerometer/y", ay);
    Firebase.RTDB.setInt(&fbdo, "/livestock/accelerometer/z", az);
    Firebase.RTDB.setFloat(&fbdo, "/livestock/movementLevel", smoothMovement);
    Firebase.RTDB.setString(&fbdo, "/livestock/motionStatus", motionState);
  }

  Serial.println("----------------------------");
  delay(1000);
}