#include "config.h"
#ifdef USE_LOCAL_CREDENTIALS
  #include "credentials_local.h"
#else
  #include "credentials.h"
#endif
#include "buttons.h"
#include "sensors.h"
#include "nixiedisplay.h"
#include "ntp.h"
#include "nixiewebserver.h"
#include "ota.h"
#include <WiFi.h>
#include <ArduinoOTA.h>
#include "display_manager.h"
#include "mqtt.h"

NixieDisplay nixieDisplay;
extern bool clockUpdateEnabled;

void setup() {
    Serial.begin(115200);

    nixieDisplay.begin();                // Initialize nixie display hardware
    displayManagerInit(nixieDisplay);    // Pass nixieDisplay to display manager

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi");

    // Blink 88:88:88 while waiting for WiFi
    while (WiFi.status() != WL_CONNECTED) {
        nixieDisplay.showNumber(888888);
        nixieDisplay.updateDisplay();
        delay(500);

        nixieDisplay.disableAllSegments();
        nixieDisplay.updateDisplay();
        delay(500);

        Serial.print(".");
    }

    Serial.println("\nWiFi connected. IP address: " + WiFi.localIP().toString());
    showIpChunks(WiFi.localIP());

    initOTA();
    initTime();

    if (!initSensors()) {
        Serial.println("Sensor initialization failed!");
    }

    initButtons();
    initWebServer();
    initMQTT();
}

void loop() {
    ArduinoOTA.handle();
    handleWebRequests();
    updateButtons();

    if (wasButtonJustPressed(BUTTON_1)) {
        Serial.println("Button 1 just pressed!");
        triggerTempHumiDisplay();
    }

    if (wasButtonJustPressed(BUTTON_3)) {
        Serial.println("Button 3 just pressed!");
        triggerSlotMachine();
    }

    static unsigned long lastTimeCheck = 0;
    unsigned long now = millis();
    if (now - lastTimeCheck >= 500) {
        lastTimeCheck = now;
        updateDisplayManager();  // periodic logic here
    }

    delay(10);  // debounce delay is separate; you can lower this too
}
