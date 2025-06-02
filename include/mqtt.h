#pragma once

#include <Arduino.h>
#include "sensors.h"

void initMQTT();
void mqttLoop();
void mqttPublishStatus(const SensorData& data);
void mqttTriggerDisplayOverride(uint32_t number, uint32_t duration = 5000);
bool isMqttDisplayOverrideActive();
void mqttPublishDiscovery();
