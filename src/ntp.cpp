#include "ntp.h"
#include "config.h"
#include <Arduino.h>

void initTime() {
  // Use the POSIX time-zone string for Switzerland:
  // Standard time is CET (UTC+1) and DST is CEST (UTC+2).
  // Transitions: DST starts on the last Sunday of March at 2:00 AM and ends on the last Sunday of October at 3:00 AM.
  configTzTime("CET-1CEST,M3.5.0/2,M10.5.0/3", NTP_PRIMARY, NTP_SECONDARY);
  Serial.println("Time synchronization started");
}

String getFormattedTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Failed to obtain time";
  }
  char buffer[30];
  strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeinfo);  // Format as HH:MM:SS.
  return String(buffer);
}
