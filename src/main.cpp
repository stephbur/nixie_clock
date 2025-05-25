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
    static bool prevButton1 = isButtonPressed(BUTTON1_PIN);
    static bool prevButton3 = isButtonPressed(BUTTON3_PIN);
    unsigned long now = millis();

    if (now - lastTimeCheck >= 500) {
        lastTimeCheck = now;

        String currentTime = getFormattedTime();
        Serial.println("Current time: " + currentTime);
        updateDisplayManager(currentTime);

        bool currButton1 = isButtonPressed(BUTTON1_PIN);
        bool currButton3 = isButtonPressed(BUTTON3_PIN);

        if (currButton1 && !prevButton1) {
            triggerTempHumiDisplay();
        }
        if (currButton3 && !prevButton3) {
            triggerSlotMachine();
        }

        prevButton1 = currButton1;
        prevButton3 = currButton3;
    }

    delay(10);
}
