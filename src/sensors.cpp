#include "sensors.h"
#include "config.h"
#include <Wire.h>
#include <Arduino.h>

Adafruit_AHTX0 aht;
Adafruit_BMP280 bmp;

bool initSensors() {
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  
  bool aht_status = aht.begin();
  bool bmp_status = bmp.begin(); // Uses default I2C address (0x76/0x77)
  
  if (!bmp_status) {
    Serial.println("Could not find BMP280 sensor!");
  }
  if (!aht_status) {
    Serial.println("Could not find AHT20 sensor!");
  }
  return aht_status && bmp_status;
}

SensorData readSensors() {
  SensorData data;
  
  // Read AHT20 data using separate events for humidity and temperature
  sensors_event_t humidity_event, temp_event;
  aht.getEvent(&humidity_event, &temp_event);
  data.temperatureAHT = temp_event.temperature;
  data.humidity = humidity_event.relative_humidity;
  
  // Read BMP280 data
  data.temperatureBMP = bmp.readTemperature();
  data.pressure = bmp.readPressure() / 100.0F; // Convert Pa to hPa
  
  return data;
}
