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
    static bool prevButton1       = isButtonPressed(BUTTON1_PIN);
    static bool prevButton3       = isButtonPressed(BUTTON3_PIN);
    unsigned long now             = millis();

    if (now - lastTimeCheck >= 1000) {
        lastTimeCheck = now;

        String currentTime = getFormattedTime();
        Serial.println("Current time: " + currentTime);

        // Only run scheduled updates if auto‐update is enabled
        if (clockUpdateEnabled) {
            updateDisplayManager(currentTime);
        }

        // Read buttons
        bool currButton1 = isButtonPressed(BUTTON1_PIN);
        bool currButton3 = isButtonPressed(BUTTON3_PIN);
        Serial.printf(
          "B1 prev=%d curr=%d | B3 prev=%d curr=%d\n",
          prevButton1, currButton1,
          prevButton3, currButton3
        );

        // Manual triggers always allowed
        if (currButton1 && !prevButton1) {
            Serial.println("✔ Button 1: Triggering sensor display");
            triggerSensorDisplay();
        }
        if (currButton3 && !prevButton3) {
            Serial.println("✔ Button 3: Triggering slot machine");
            triggerSlotMachine();
        }

        prevButton1 = currButton1;
        prevButton3 = currButton3;

        // Show the time on the nixie only if no override is active and auto-update is on
        if (clockUpdateEnabled && !isDisplayOverrideActive()) {
            int h, m, s;
            if (sscanf(currentTime.c_str(), "%d:%d:%d", &h, &m, &s) == 3) {
                uint32_t compactTime = h * 10000 + m * 100 + s;
                nixieDisplay.showNumber(compactTime);
            }
        }
    }

    delay(10);
}