#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <MPU6050.h>
#include <TinyGPS++.h>

// ================= WIFI =================
#define WIFI_SSID "Naveesha's A56"
#define WIFI_PASSWORD "nb2k6900"

// ================= FIREBASE =================
#define API_KEY "AIzaSyAu7nrubc0mqtKsmA22mgLLpQkYAkAEy74"
#define DATABASE_URL "https://livestock-monitoring-sys-ad970-default-rtdb.asia-southeast1.firebasedatabase.app/"

// ================= DS18B20 =================
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// ================= MPU6050 =================
MPU6050 mpu;

// ================= GPS =================
TinyGPSPlus gps;
#define RXD2 16
#define TXD2 17

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;

// Variables
float temperatureC;
String healthStatus;
String activityStatus;

float latitude = 0;
float longitude = 0;

void setup() {

  Serial.begin(115200);

  // WIFI CONNECT
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.println("WiFi Connected");

  // DS18B20 START
  sensors.begin();

  // MPU6050 START
  Wire.begin(21, 22);
  mpu.initialize();

  if (mpu.testConnection()) {
    Serial.println("MPU6050 Connected");
  } else {
    Serial.println("MPU6050 Error");
  }

  // GPS START
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial.println("GPS Ready");

  // FIREBASE SETUP
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase SignUp OK");
    signupOK = true;
  } 
  else {
    Serial.printf("Firebase Error: %s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("Firebase Ready");
}

void loop() {

  // ================= TEMPERATURE =================
  sensors.requestTemperatures();
  temperatureC = sensors.getTempCByIndex(0);

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

  if (accelMagnitude < 1.05) {
    activityStatus = "Resting";
  }
  else if (accelMagnitude < 1.5) {
    activityStatus = "Walking";
  }
  else {
    activityStatus = "Running";
  }

  // ================= GPS =================
  while (Serial2.available() > 0) {

    gps.encode(Serial2.read());

    if (gps.location.isUpdated()) {
      latitude = gps.location.lat();
      longitude = gps.location.lng();
    }
  }

  // ================= SERIAL MONITOR =================
  Serial.println("----- Animal Status -----");

  Serial.print("Temperature: ");
  Serial.println(temperatureC);

  Serial.print("Health: ");
  Serial.println(healthStatus);

  Serial.print("Activity: ");
  Serial.println(activityStatus);

  Serial.print("Latitude: ");
  Serial.println(latitude, 6);

  Serial.print("Longitude: ");
  Serial.println(longitude, 6);

  Serial.println("-------------------------");

  // ================= FIREBASE SEND =================
  if (Firebase.ready() && signupOK) {

    Firebase.RTDB.setFloat(&fbdo, "/livestock/temperature", temperatureC);

    Firebase.RTDB.setString(&fbdo, "/livestock/healthStatus", healthStatus);

    Firebase.RTDB.setString(&fbdo, "/livestock/activity", activityStatus);

    Firebase.RTDB.setFloat(&fbdo, "/livestock/gps/latitude", latitude);

    Firebase.RTDB.setFloat(&fbdo, "/livestock/gps/longitude", longitude);

    Serial.println("Data Sent To Firebase");
  }

  delay(5000);
}