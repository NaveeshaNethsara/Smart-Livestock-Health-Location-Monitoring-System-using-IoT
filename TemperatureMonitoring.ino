//Temperature Monitoring
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 4   // DS18B20 connected to GPIO 4

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Threshold values
float feverThreshold = 39.5;
float lowTempThreshold = 37.5;

void setup() {
  Serial.begin(115200);
  sensors.begin();
  Serial.println("Animal Body Temperature Monitoring Started...");
}

void loop() {

  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);

  Serial.print("Body Temperature: ");
  Serial.print(temperatureC);
  Serial.println(" °C");

  // -------- Disease Detection Logic --------
  if (temperatureC > feverThreshold) {
    Serial.println("⚠ ALERT: Possible Fever Detected!");
  }
  else if (temperatureC < lowTempThreshold) {
    Serial.println("⚠ ALERT: Possible Hypothermia Detected!");
  }
  else {
    Serial.println("✅ Temperature Normal - Animal Healthy");
  }

  Serial.println("----------------------------");
  delay(5000);  // Read every 5 seconds
}