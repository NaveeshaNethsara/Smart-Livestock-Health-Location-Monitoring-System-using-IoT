#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <TinyGPS++.h>

// WIFI 
#define WIFI_SSID "Naveesha's A56"
#define WIFI_PASSWORD "nb2k6900"


#define API_KEY "AIzaSyAu7nrubc0mqtKsmA22mgLLpQkYAkAEy74"
#define DATABASE_URL "livestock-monitoring-sys-ad970-default-rtdb.asia-southeast1.firebasedatabase.app"


#define RXD2 16
#define TXD2 17

TinyGPSPlus gps;

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void setup() {

  Serial.begin(115200);
  delay(1000);

  // Start GPS serial
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  // WiFi Connection
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nWiFi Connected");

  // Firebase configuration
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  config.signer.test_mode = true;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("Firebase Ready");
  Serial.println("GPS Module Ready");
}

void loop() {

  while (Serial2.available() > 0) {

    char c = Serial2.read();
    gps.encode(c);

    if (gps.location.isUpdated()) {

      float latitude = gps.location.lat();
      float longitude = gps.location.lng();

      Serial.println("---- GPS DATA ----");
      Serial.print("Latitude: ");
      Serial.println(latitude, 6);

      Serial.print("Longitude: ");
      Serial.println(longitude, 6);

      Serial.println("------------------");

      // Send to Firebase
      Firebase.RTDB.setFloat(&fbdo, "/livestock/gps/latitude", latitude);
      Firebase.RTDB.setFloat(&fbdo, "/livestock/gps/longitude", longitude);

      Serial.println("Location Sent To Firebase");

    }
  }

}
