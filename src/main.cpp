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
#include "webserver.h"
#include "ota.h"
#include <WiFi.h>
#include <ArduinoOTA.h>
#include "display_manager.h"


NixieDisplay nixieDisplay;
extern bool clockUpdateEnabled;

void setup() {
  Serial.begin(115200);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected. IP address: " + WiFi.localIP().toString());
  
  initOTA();
  
  initTime();
  
  if (!initSensors()) {
    Serial.println("Sensor initialization failed!");
  }
  
  nixieDisplay.begin();
  displayManagerInit(nixieDisplay);

  
  initButtons();
  
  initWebServer();
}

void loop() {
    ArduinoOTA.handle();
    handleWebRequests();
    updateButtons();

    static unsigned long lastTimeCheck = 0;
    unsigned long now = millis();

    if (now - lastTimeCheck >= 1000) {
        lastTimeCheck = now;
        String currentTime = getFormattedTime();
        Serial.println("Current time: " + currentTime);

        if (clockUpdateEnabled) {
            updateDisplayManager(currentTime);
        }

        if (clockUpdateEnabled && !isDisplayOverrideActive()) {
            int h, m, s;
            if (sscanf(currentTime.c_str(), "%d:%d:%d", &h, &m, &s) == 3) {
                nixieDisplay.showNumber(h * 10000 + m * 100 + s);
            }
        }
    }

    if (isButtonPressed(BUTTON1_PIN)) {
        Serial.println("✔ Button 1 pressed → sensor display");
        triggerSensorDisplay();
        while (isButtonPressed(BUTTON1_PIN)) {
            delay(10);
        }
    }
    if (isButtonPressed(BUTTON3_PIN)) {
        Serial.println("✔ Button 3 pressed → slot machine");
        triggerSlotMachine();
        while (isButtonPressed(BUTTON3_PIN)) {
            delay(10);
        }
    }

    delay(10);
}