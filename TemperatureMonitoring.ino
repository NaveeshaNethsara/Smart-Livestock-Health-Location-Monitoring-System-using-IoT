#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 4  // GPIO 4

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(115200);
  sensors.begin();
  Serial.println("DS18B20 Temperature Sensor Test");
}

void loop() {
  sensors.requestTemperatures();
  
  float temperatureC = sensors.getTempCByIndex(0);

  Serial.print("Temperature: ");
  Serial.print(temperatureC);
  Serial.println(" °C");

  // Basic condition check
  if (temperatureC > 39.5) {
    Serial.println("⚠ Fever Detected!");
  } else if (temperatureC < 35) {
    Serial.println("⚠ Low Temperature!");
  } else {
    Serial.println("✅ Normal Temperature");
  }

  Serial.println("--------------------");

  delay(2000);
}
