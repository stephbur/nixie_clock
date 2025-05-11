#ifndef NTP_H
#define NTP_H

#include <Arduino.h>
#include <time.h>

// Initialize time synchronization using NTP
void initTime();

// Get the current time as a formatted string
String getFormattedTime();

#endif // NTP_H
