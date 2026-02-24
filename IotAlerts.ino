#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <TinyGPS++.h>


const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";


String animalName = "Cow-01";


float SAFE_MIN_LAT = 6.9200;
float SAFE_MAX_LAT = 6.9350;
float SAFE_MIN_LON = 79.8500;
float SAFE_MAX_LON = 79.8700;


#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
float MAX_TEMP = 39.0;


TinyGPSPlus gps;
HardwareSerial gpsSerial(1);


WebServer server(80);

//HTML 
String webpage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<title>Livestock Monitoring</title>
<style>
body { font-family: Arial; text-align: center; }
.alert { color: red; font-weight: bold; }
</style>
</head>
<body>

<h1>Livestock Monitoring System</h1>
<h2 id="name"></h2>
<p>Temperature: <span id="temp"></span> °C</p>
<p>Latitude: <span id="lat"></span></p>
<p>Longitude: <span id="lon"></span></p>

<script>
const SAFE_MIN_LAT = 6.9200;
const SAFE_MAX_LAT = 6.9350;
const SAFE_MIN_LON = 79.8500;
const SAFE_MAX_LON = 79.8700;
const MAX_TEMP = 39.0;

let outAlert = false;
let tempAlert = false;

function fetchData() {
    fetch("/data")
    .then(response => response.json())
    .then(data => {

        document.getElementById("name").innerText = data.name;
        document.getElementById("temp").innerText = data.temperature;
        document.getElementById("lat").innerText = data.latitude;
        document.getElementById("lon").innerText = data.longitude;

        const lat = data.latitude;
        const lon = data.longitude;
        const temp = data.temperature;
        const name = data.name;

        const outOfRange =
            lat < SAFE_MIN_LAT ||
            lat > SAFE_MAX_LAT ||
            lon < SAFE_MIN_LON ||
            lon > SAFE_MAX_LON;

        if (outOfRange && !outAlert) {
            alert("⚠ ALERT: " + name + " is OUT OF RANGE!");
            outAlert = true;
        }

        if (!outOfRange) outAlert = false;

        if (temp > MAX_TEMP && !tempAlert) {
            alert("🔥 WARNING: " + name + "'s temperature is too HIGH!");
            tempAlert = true;
        }

        if (temp <= MAX_TEMP) tempAlert = false;
    });
}

setInterval(fetchData, 5000);
</script>

</body>
</html>
)rawliteral";


void handleRoot() {
  server.send(200, "text/html", webpage);
}

void handleData() {
  float temperature = dht.readTemperature();
  float latitude = gps.location.lat();
  float longitude = gps.location.lng();

  if (isnan(temperature)) temperature = 0;
  if (!gps.location.isValid()) {
    latitude = 0;
    longitude = 0;
  }

  String json = "{";
  json += "\"name\":\"" + animalName + "\",";
  json += "\"temperature\":" + String(temperature) + ",";
  json += "\"latitude\":" + String(latitude, 6) + ",";
  json += "\"longitude\":" + String(longitude, 6);
  json += "}";

  server.send(200, "application/json", json);
}


void setup() {
  Serial.begin(115200);

  dht.begin();

  gpsSerial.begin(9600, SERIAL_8N1, 16, 17); 

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
}

void loop() {

  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  server.handleClient();
}