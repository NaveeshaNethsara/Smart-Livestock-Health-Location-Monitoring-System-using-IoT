#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ================= WIFI =================
#define WIFI_SSID "Naveesha's A56"
#define WIFI_PASSWORD "nb2k6900"

// ================= FIREBASE =================
// DO NOT add https://
// DO NOT add trailing /
#define API_KEY "AIzaSyAu7nrubc0mqtKsmA22mgLLpQkYAkAEy74"
#define DATABASE_URL "livestock-monitoring-sys-ad970-default-rtdb.asia-southeast1.firebasedatabase.app"

// ================= SENSOR =================
#define ONE_WIRE_BUS 4

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void setup() {

  Serial.begin(115200);
  delay(1000);

  // Connect WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nWiFi Connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // Start temperature sensor
  sensors.begin();

  // Firebase configuration
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Anonymous login
  config.signer.test_mode = true;

  // Optional but improves stability
  config.timeout.serverResponse = 10000;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("Firebase Initialized");
}

void loop() {

  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);

  Serial.print("Temperature: ");
  Serial.println(temperatureC);

  if (Firebase.RTDB.setFloat(&fbdo, "/livestock/temperature", temperatureC)) {
    Serial.println("✅ Data sent to Firebase");
  } else {
    Serial.print("❌ Failed: ");
    Serial.println(fbdo.errorReason());
  }

  delay(3000);
}
