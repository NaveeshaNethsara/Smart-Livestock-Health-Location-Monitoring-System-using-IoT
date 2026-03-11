#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <MPU6050.h>
#include <TinyGPS++.h>
#include <time.h>
#include <math.h>

// ================= WIFI =================
#define WIFI_SSID "Naveesha's A56"
#define WIFI_PASSWORD "nb2k6900"

// ================= FIREBASE =================
#define API_KEY "AIzaSyAu7nrubc0mqtKsmA22mgLLpQkYAkAEy74"
#define DATABASE_URL "https://livestock-monitoring-sys-ad970-default-rtdb.asia-southeast1.firebasedatabase.app/"

// ================= DEVICE INFO =================
String DEVICE_ID = "DEV001";   // must match profile/deviceId in Firebase exactly
String matchedAnimalId = "";

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

// ================= FIREBASE OBJECTS =================
FirebaseData fbdo;
FirebaseData searchFbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;

float temperatureC = 0;
String healthStatus = "Pending";
String activityStatus = "Pending";
float latitude = 0;
float longitude = 0;

// ================= TIME =================
bool waitForTimeSync(unsigned long timeoutMs = 10000) {
  unsigned long start = millis();
  time_t now;

  while (millis() - start < timeoutMs) {
    time(&now);
    if (now > 100000) {
      Serial.println("Time synced");
      return true;
    }
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nTime sync timeout");
  return false;
}

// ================= FIND ANIMAL BY DEVICE ID =================
bool findAnimalByDeviceId() {
  Serial.println("Searching animal by device ID...");
  Serial.print("This ESP32 DEVICE_ID = ");
  Serial.println(DEVICE_ID);

  if (!Firebase.RTDB.getJSON(&searchFbdo, "/animals")) {
    Serial.println("Failed to read /animals");
    Serial.println(searchFbdo.errorReason());
    return false;
  }

  FirebaseJson *json = searchFbdo.to<FirebaseJson *>();
  if (json == nullptr) {
    Serial.println("JSON pointer is null");
    return false;
  }

  size_t count = json->iteratorBegin();
  String path, value;
  int type = 0;
  bool found = false;

  for (size_t i = 0; i < count; i++) {
    json->iteratorGet(i, type, path, value);

    // We only want top-level animal IDs like ANM001
    if (path.indexOf("/") == -1) {
      String animalId = path;
      FirebaseJsonData result;

      String devicePath = "/" + animalId + "/profile/deviceId";

      if (json->get(result, devicePath)) {
        String dbDeviceId = result.to<String>();
        dbDeviceId.trim();

        Serial.print("Checking animal ");
        Serial.print(animalId);
        Serial.print(" -> deviceId = ");
        Serial.println(dbDeviceId);

        if (dbDeviceId == DEVICE_ID) {
          matchedAnimalId = animalId;
          found = true;
          Serial.print("Matched animal: ");
          Serial.println(matchedAnimalId);
          break;
        }
      }
    }
  }

  json->iteratorEnd();

  if (!found) {
    Serial.println("No animal matched this device ID");
  }

  return found;
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  delay(1000);

  // WIFI
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println("WiFi Connected");
  Serial.println(WiFi.localIP());

  // DS18B20
  sensors.begin();
  Serial.println("DS18B20 Ready");

  // MPU6050
  Wire.begin(21, 22);
  Wire.setClock(400000);
  mpu.initialize();

  if (mpu.testConnection()) {
    Serial.println("MPU6050 Connected");
  } else {
    Serial.println("MPU6050 Error");
    Serial.println("Check MPU6050 wiring: VCC, GND, SDA->21, SCL->22");
  }

  // GPS
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial.println("GPS Ready");

  // FIREBASE
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase SignUp OK");
    signupOK = true;
  } else {
    Serial.print("Firebase Error: ");
    Serial.println(config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Serial.println("Firebase Ready");

  // TIME
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  waitForTimeSync();

  // FIND MATCHED ANIMAL
  if (Firebase.ready() && signupOK) {
    findAnimalByDeviceId();
  }
}

// ================= LOOP =================
void loop() {
  if (!Firebase.ready() || !signupOK) {
    Serial.println("Firebase not ready");
    delay(2000);
    return;
  }

  if (matchedAnimalId == "") {
    if (!findAnimalByDeviceId()) {
      delay(3000);
      return;
    }
  }

  // ================= TEMPERATURE =================
  sensors.requestTemperatures();
  temperatureC = sensors.getTempCByIndex(0);

  Serial.print("Raw Temperature = ");
  Serial.println(temperatureC);

  if (temperatureC == DEVICE_DISCONNECTED_C) {
    healthStatus = "Sensor Error";
  } else if (temperatureC > 39.5) {
    healthStatus = "Fever";
  } else if (temperatureC < 36.0) {
    healthStatus = "Low Temperature";
  } else {
    healthStatus = "Normal";
  }

  // ================= MOTION =================
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  float accelMagnitude = sqrt((float)ax * ax + (float)ay * ay + (float)az * az) / 16384.0;

  if (accelMagnitude < 1.05) {
    activityStatus = "Resting";
  } else if (accelMagnitude < 1.5) {
    activityStatus = "Walking";
  } else {
    activityStatus = "Running";
  }

  // ================= GPS =================
  while (Serial2.available() > 0) {
    gps.encode(Serial2.read());
  }

  if (gps.location.isValid()) {
    latitude = gps.location.lat();
    longitude = gps.location.lng();
  }

  // ================= TIME =================
  time_t now;
  time(&now);

  // if time still not synced, use millis/1000 fallback
  if (now < 100000) {
    now = millis() / 1000;
  }

  // ================= FIREBASE SEND =================
  String basePath = "/animals/" + matchedAnimalId + "/live";

  bool ok = true;

  ok &= Firebase.RTDB.setFloat(&fbdo, basePath + "/temperature", temperatureC);
  if (!ok) Serial.println(fbdo.errorReason());

  ok &= Firebase.RTDB.setString(&fbdo, basePath + "/healthStatus", healthStatus);
  if (!ok) Serial.println(fbdo.errorReason());

  ok &= Firebase.RTDB.setString(&fbdo, basePath + "/activity", activityStatus);
  if (!ok) Serial.println(fbdo.errorReason());

  ok &= Firebase.RTDB.setFloat(&fbdo, basePath + "/gps/latitude", latitude);
  if (!ok) Serial.println(fbdo.errorReason());

  ok &= Firebase.RTDB.setFloat(&fbdo, basePath + "/gps/longitude", longitude);
  if (!ok) Serial.println(fbdo.errorReason());

  ok &= Firebase.RTDB.setInt(&fbdo, basePath + "/lastUpdated", now);
  if (!ok) Serial.println(fbdo.errorReason());

  ok &= Firebase.RTDB.setBool(&fbdo, basePath + "/online", true);
  if (!ok) Serial.println(fbdo.errorReason());

  // ================= SERIAL MONITOR =================
  Serial.println("----- Animal Status -----");
  Serial.print("Matched Animal ID: ");
  Serial.println(matchedAnimalId);

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

  Serial.print("LastUpdated: ");
  Serial.println((unsigned long)now);

  Serial.println(ok ? "Data Sent To Firebase" : "Firebase Write Error");
  Serial.println("-------------------------");

  delay(5000);
}