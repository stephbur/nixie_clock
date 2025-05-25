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
#include "mqtt.h"

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
