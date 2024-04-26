#include "arduino_secrets.h"
#include <WiFi.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <Wire.h>
#include <RTClib.h>

#include "thingProperties.h"

// Replace these values with your actual WiFi credentials
#define WIFI_SSID "Fios-MYKV7"
#define WIFI_PASSWORD "true070zulu22frank"

// Replace these values with your InfluxDB Cloud credentials
#define INFLUXDB_URL "https://us-east-1-1.aws.cloud2.influxdata.com"
#define INFLUXDB_TOKEN "mFidyPKstjiFsNEAEptbWt7MSeb_fN8KAhDcTZ-1Ish5Dtrs_ukWUof3AZ7R_MDF2DvNfGvcBrGG_jJS1RNC9A=="
#define INFLUXDB_ORG "5fb88f22f8696065"
#define INFLUXDB_BUCKET "Ocean_Data"

// Set timezone string according to https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
#define TZ_INFO "EST5EDT,M3.2.0,M11.1.0"

#define ONE_WIRE_BUS 22
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

#define DHTPIN 23
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

RTC_DS3231 rtc;

const int RTC_DAT_PIN = 21; // DAT pin connected to D21
const int RTC_RST_PIN = 19; // RST pin connected to D19
const int RTC_CLK_PIN = 18; // CLK pin connected to D18

InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

Point sensor("wifi_status");
Point sensorDHT("sensor_data");

void setup() {
  Serial.begin(115200);

  // Setup wifi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();

  // Add tags
  sensor.addTag("device", "ESP32");
  sensor.addTag("SSID", WiFi.SSID());

  // Initialize RTC with custom pin connections
  Wire.begin(RTC_DAT_PIN, RTC_CLK_PIN); // SDA/DAT pin, SCL/CLK pin
  rtc.begin();

  pinMode(RTC_RST_PIN, OUTPUT); // Set RST pin as output
  digitalWrite(RTC_RST_PIN, LOW); // Reset RTC
  delay(10); // Short delay
  digitalWrite(RTC_RST_PIN, HIGH); // Release RTC reset

  DateTime compiled = DateTime(__DATE__, __TIME__);
  if (rtc.now() < compiled) {
    Serial.println("RTC is NOT running");
  } else {
    Serial.println("RTC is running");
  }

  // Accurate time is necessary for certificate validation and writing in batches
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  sensors.begin();
  dht.begin();
  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
}

void loop() {
  ArduinoCloud.update();
  readDHTSensor();
  sensors.requestTemperatures();
  float waterTemp = sensors.getTempFByIndex(0);
  water_temperature = waterTemp;

  DateTime now = rtc.now(); // Get the current time from the RTC module
  
  Serial.print("RTC Time: ");
  Serial.print(now.year());
  Serial.print('/');
  Serial.print(now.month());
  Serial.print('/');
  Serial.print(now.day());
  Serial.print(' ');
  Serial.print(now.hour());
  Serial.print(':');
  Serial.print(now.minute());
  Serial.print(':');
  Serial.print(now.second());
  Serial.println();

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print("°C Humidity: ");
  Serial.print(humidity);
  Serial.print("% Water Temperature: ");
  Serial.print(waterTemp);
  Serial.println("°F");

  // Update cloud properties
  water_temperature = waterTemp;
  temperature = temperature;
  humidity = humidity;
  
  // Wait 10s
  Serial.println("Wait 10s");
  delay(10000);
}

void readDHTSensor() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  temperature = t;
  humidity = h;
  delay(10000);
}

void onRealTimeClockChange() {
  // Add your code here to act upon RealTimeClock change
}
