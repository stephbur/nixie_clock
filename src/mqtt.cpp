#include "mqtt.h"
#include "nixiedisplay.h"
#include <WiFi.h>
#include <PubSubClient.h>
#ifdef USE_LOCAL_CREDENTIALS
  #include "credentials_local.h"
#else
  #include "credentials.h"
#endif
#include "ntp.h"
#ifdef MQTT_ENABLED

WiFiClient espClient;
PubSubClient mqttClient(espClient);
unsigned long lastReconnectAttempt = 0;
unsigned long displayOverrideUntil = 0;

extern NixieDisplay nixieDisplay;

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    payload[length] = '\0';
    String message = String((char*)payload);

    Serial.println("MQTT callback triggered");
    Serial.print("  Topic: ");
    Serial.println(topic);
    Serial.print("  Message: ");
    Serial.println(message);

    if (String(topic) == MQTT_TOPIC_COMMAND && message.length() == 6 && message.toInt() >= 0) {
        uint32_t number = message.toInt();
        Serial.print("  Valid command received, displaying: ");
        Serial.println(number);
        mqttTriggerDisplayOverride(number);
    } else {
        Serial.println("  Invalid MQTT command or format.");
    }
}

bool mqttReconnect() {
    Serial.print("Attempting MQTT connection to ");
    Serial.print(MQTT_BROKER);
    Serial.print(":");
    Serial.println(MQTT_PORT);

    if (mqttClient.connect("NixieClock", MQTT_USER, MQTT_PASSWORD)) {
        Serial.println("MQTT connected successfully.");
        mqttClient.subscribe(MQTT_TOPIC_COMMAND);
        Serial.print("Subscribed to command topic: ");
        Serial.println(MQTT_TOPIC_COMMAND);
        mqttPublishDiscovery();
        return true;
    } else {
        Serial.print("MQTT connection failed. rc=");
        Serial.println(mqttClient.state());
        return false;
    }
}

void mqttPublishDiscovery() {
    const char* deviceId = "nixie_sensor_node";
    const char* deviceName = "Nixie Clock";

    struct {
        const char* name;
        const char* id;
        const char* topic;
        const char* unit;
        const char* device_class;
        const char* state_class;
    } sensors[] = {
        { "Nixie Temperature", "nixie_temp", "sensor/nixie/temp", "°C", "temperature", "measurement" },
        { "Nixie Humidity",    "nixie_humi", "sensor/nixie/humi", "%", "humidity", "measurement" },
        { "Nixie Pressure",    "nixie_pressure", "sensor/nixie/pressure", "hPa", "pressure", "measurement" }
    };

    for (auto& s : sensors) {
        String configTopic = "homeassistant/sensor/" + String(s.id) + "/config";

        String payload = "{";
        payload += "\"name\":\"" + String(s.name) + "\",";
        payload += "\"state_topic\":\"" + String(s.topic) + "\",";
        payload += "\"unit_of_measurement\":\"" + String(s.unit) + "\",";
        payload += "\"device_class\":\"" + String(s.device_class) + "\",";
        payload += "\"state_class\":\"" + String(s.state_class) + "\",";
        payload += "\"value_template\":\"{{ value | float }}\",";
        payload += "\"unique_id\":\"" + String(s.id) + "\",";
        payload += "\"device\":{";
        payload +=   "\"identifiers\":[\"" + String(deviceId) + "\"],";
        payload +=   "\"name\":\"" + String(deviceName) + "\",";
        payload +=   "\"manufacturer\":\"Custom\",";
        payload +=   "\"model\":\"Nixie MQTT Node\"";
        payload += "}";
        payload += "}";

        Serial.println("Publishing discovery config to " + configTopic);
        mqttClient.publish(configTopic.c_str(), payload.c_str(), true);
    }
}


void initMQTT() {
    Serial.println("Initializing MQTT...");
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);
    mqttPublishDiscovery();
}

void mqttLoop() {
    if (!mqttClient.connected()) {
        unsigned long now = millis();
        if (now - lastReconnectAttempt > 5000) {
            Serial.println("MQTT not connected, trying to reconnect...");
            lastReconnectAttempt = now;
            if (mqttReconnect()) {
                lastReconnectAttempt = 0;
            }
        }
    } else {
        mqttClient.loop();
    }
}

void mqttPublishStatus(const SensorData& data) {
    mqttClient.publish("sensor/nixie/temp", String(data.temperatureAHT, 2).c_str(), true);
    mqttClient.publish("sensor/nixie/humi", String(data.humidity, 2).c_str(), true);
    mqttClient.publish("sensor/nixie/pressure", String(data.pressure, 2).c_str(), true);

    Serial.println("Published MQTT sensor/nixie/temp: " + String(data.temperatureAHT, 2));
    Serial.println("Published MQTT sensor/nixie/humi: " + String(data.humidity, 2));
    Serial.println("Published MQTT sensor/nixie/pressure: " + String(data.pressure, 2));
}

void mqttTriggerDisplayOverride(uint32_t number, uint32_t duration) {
    Serial.print("MQTT override triggered. Number: ");
    Serial.print(number);
    Serial.print(", Duration: ");
    Serial.println(duration);

    nixieDisplay.showNumber(number);
    nixieDisplay.updateDisplay();
    displayOverrideUntil = millis() + duration;
}

bool isMqttDisplayOverrideActive() {
    return millis() < displayOverrideUntil;
}

#else

void mqttInit() {}
void mqttLoop() {}
void mqttPublishStatus(const String&, const String&) {}
void mqttTriggerDisplayOverride(uint32_t, uint32_t) {}
bool isMqttDisplayOverrideActive() { return false; }

#endif
