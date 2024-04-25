#include "thingProperties.h"
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define DHTPIN 8        // Pin connected to the DHT11 sensor
#define DHTTYPE DHT11   // DHT sensor type

DHT dht(DHTPIN, DHTTYPE); // Initialize the DHT sensor

#define ONE_WIRE_BUS 9   // Pin connected to the DS18B20 temperature sensor
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

#define sensorPin A0

void setup() {
  Serial.begin(9600);
  dht.begin();
  sensors.begin();  // Initialize DS18B20 temperature sensor
  initProperties(); // Initialize Arduino IoT Cloud properties
  ArduinoCloud.begin(ArduinoIoTPreferredConnection); // Initialize Arduino IoT Cloud library
  setDebugMessageLevel(DBG_INFO);
  ArduinoCloud.printDebugInfo();
}

void loop() {
  ArduinoCloud.update();

  // Read temperature and humidity from DHT sensor
  float temperatureValue = dht.readTemperature();
  float humidityValue = dht.readHumidity();

  // Read TDS sensor data
  int tdsValue = analogRead(sensorPin);
  float Voltage = tdsValue * 5 / 1024.0; // Convert analog reading to Voltage
  float tds = (133.42 / Voltage * Voltage * Voltage - 255.86 * Voltage * Voltage + 857.39 * Voltage) * 0.5; // Convert voltage value to TDS value

  // Read DS18B20 temperature sensor data
  sensors.requestTemperatures();
  float waterTempValue = sensors.getTempCByIndex(0);

  // Update Arduino IoT Cloud properties
  temperature = temperatureValue;
  humidity = humidityValue;
  tDS = tds;
  waterTemp = waterTempValue;

  // Print data to serial monitor
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C\tHumidity: ");
  Serial.print(humidity);
  Serial.print(" %\tTDS: ");
  Serial.print(tDS);
  Serial.print(" ppm\tWater Temperature: ");
  Serial.print(waterTemp);
  Serial.println(" °C");

  delay(1000); // Adjust delay as needed
}
