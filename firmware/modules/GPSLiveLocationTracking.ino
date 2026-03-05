#include <TinyGPS++.h>
#include <HardwareSerial.h>

TinyGPSPlus gps;

// Use UART2 on ESP32
HardwareSerial gpsSerial(2);

void setup() {
  Serial.begin(115200);
  gpsSerial.begin(9600, SERIAL_8N1, 16, 17); // RX=16, TX=17

  Serial.println("ESP32 GPS Tracker Started...");
}

void loop() {
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());

    if (gps.location.isUpdated()) {
      double lat = gps.location.lat();
      double lng = gps.location.lng();

      Serial.println("----- GPS Data -----");
      Serial.print("Latitude: "); Serial.println(lat, 6);
      Serial.print("Longitude: "); Serial.println(lng, 6);
      Serial.print("Satellites: "); Serial.println(gps.satellites.value());
      Serial.print("Altitude (m): "); Serial.println(gps.altitude.meters());
      Serial.print("Speed (km/h): "); Serial.println(gps.speed.kmph());

      // Google Maps link
      Serial.print("Google Maps: ");
      Serial.print("https://www.google.com/maps?q=");
      Serial.print(lat, 6);
      Serial.print(",");
      Serial.println(lng, 6);

      Serial.println("--------------------\n");
    }
  }
}
