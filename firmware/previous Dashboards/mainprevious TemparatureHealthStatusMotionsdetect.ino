#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <MPU6050.h>

// ================= WIFI =================
#define WIFI_SSID "Naveesha's A56"
#define WIFI_PASSWORD "nb2k6900"

// ================= FIREBASE =================
#define API_KEY "AIzaSyAu7nrubc0mqtKsmA22mgLLpQkYAkAEy74"
#define DATABASE_URL "livestock-monitoring-sys-ad970-default-rtdb.asia-southeast1.firebasedatabase.app"

// ================= DS18B20 =================
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// ================= MPU6050 =================
MPU6050 mpu;

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variables
float temperatureC;
String activityStatus;
String healthStatus;

void setup() {

  Serial.begin(115200);
  delay(1000);

  // WiFi Connection
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nWiFi Connected");

  // Initialize DS18B20
  sensors.begin();

  // Initialize MPU6050
  Wire.begin(21, 22);
  mpu.initialize();

  if (mpu.testConnection()) {
    Serial.println("MPU6050 Connected");
  } else {
    Serial.println("MPU6050 Connection Failed");
  }

  // Firebase configuration
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  config.signer.test_mode = true;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("Firebase Ready");
}

void loop() {

  // ================= TEMPERATURE =================
  sensors.requestTemperatures();
  temperatureC = sensors.getTempCByIndex(0);

  // Health Rule Engine
  if (temperatureC > 39.5) {
    healthStatus = "Fever";
  } 
  else if (temperatureC < 36.0) {
    healthStatus = "Low Temperature";
  } 
  else {
    healthStatus = "Normal";
  }

  // ================= MOTION =================
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  float accelMagnitude = sqrt(ax * ax + ay * ay + az * az) / 16384.0;

  // Simple Activity Classification
  if (accelMagnitude < 1.05) {
    activityStatus = "Resting";
  } 
  else if (accelMagnitude < 1.5) {
    activityStatus = "Walking";
  } 
  else {
    activityStatus = "Running";
  }

  // ================= SERIAL OUTPUT =================
  Serial.println("---- Animal Status ----");
  Serial.print("Temperature: ");
  Serial.println(temperatureC);
  Serial.print("Health: ");
  Serial.println(healthStatus);
  Serial.print("Activity: ");
  Serial.println(activityStatus);
  Serial.println("-----------------------");

  // ================= SEND TO FIREBASE =================

  Firebase.RTDB.setFloat(&fbdo, "/livestock/temperature", temperatureC);
  Firebase.RTDB.setString(&fbdo, "/livestock/healthStatus", healthStatus);
  Firebase.RTDB.setString(&fbdo, "/livestock/activity", activityStatus);

  delay(5000);
}
