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

#ifdef USE_LOCAL_CREDENTIALS
  #pragma message("⚠️ Using local credentials!")
#else
  #pragma message("✅ Using generic credentials")
#endif

NixieDisplay nixieDisplay;

void setup() {
  Serial.begin(115200);
  
  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected. IP address: " + WiFi.localIP().toString());
  
  // Future implementation: On first boot, start in AP mode to allow setting SSID/password.
  // For now, this is only indicated as a comment:
  /*
  if (firstBoot()) {
    // Start AP mode and allow configuration of WiFi credentials.
  }
  */
  
  // Initialize OTA updates
  initOTA();
  
  // Initialize NTP time synchronization
  initTime();
  
  // Initialize sensors
  if (!initSensors()) {
    Serial.println("Sensor initialization failed!");
  }
  
  // Initialize shift registers / Nixie display
  nixieDisplay.begin();
  
  // Initialize buttons
  initButtons();
  
  // Initialize web server
  initWebServer();
}

void loop() {
  ArduinoOTA.handle();
  handleWebRequests();

  static unsigned long lastTimeCheck = 0;
  static unsigned long lastSlotMachineRun = 0;

  unsigned long now = millis();

  // Update display every second
  if (now - lastTimeCheck >= 1000) {
    lastTimeCheck = now;

    String currentTime = getFormattedTime();
    Serial.println("Current time: " + currentTime);

    // Parse the HHMMSS as a number to display
    int h, m, s;
    if (sscanf(currentTime.c_str(), "%d:%d:%d", &h, &m, &s) == 3) {
      uint32_t compactTime = h * 10000 + m * 100 + s;
      nixieDisplay.showNumber(compactTime);
    }
  }

  // Run slot machine every 5 minutes
//   if (m%5==4) { // 300,000 ms = 5 minutes
//     nixieDisplay.runSlotMachine();
//   }

  delay(10);
}
