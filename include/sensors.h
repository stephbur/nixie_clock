#ifndef SENSORS_H
#define SENSORS_H

#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>

// Struct to hold sensor data readings
struct SensorData {
  float temperatureAHT;
  float humidity;
  float temperatureBMP;
  float pressure;
};

// Initialize both sensors (AHT20 and BMP280)
bool initSensors();

// Read both sensors and return a SensorData struct
SensorData readSensors();

#endif // SENSORS_H
