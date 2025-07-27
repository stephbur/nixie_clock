#pragma once

#include <Arduino.h>
#include "sensors.h"

#ifdef MQTT_ENABLED

void initMQTT();
void mqttLoop();
void mqttPublishStatus(const SensorData& data);
void mqttTriggerDisplayOverride(uint32_t number, uint32_t duration = 5000);
bool isMqttDisplayOverrideActive();
void mqttPublishDiscovery();

#else

inline void initMQTT() {}
inline void mqttLoop() {}
inline void mqttPublishStatus(const SensorData&) {}
inline void mqttTriggerDisplayOverride(uint32_t, uint32_t = 5000) {}
inline bool isMqttDisplayOverrideActive() { return false; }
inline void mqttPublishDiscovery() {}

#endif
