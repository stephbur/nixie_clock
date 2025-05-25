#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>

class NixieDisplay;

void displayManagerInit(NixieDisplay& display);
void updateDisplayManager(const String& currentTime);
void triggerTempHumiDisplay();         // externally triggered (e.g. by time or button)
void triggerSlotMachine();           // externally triggered (e.g. by time or button)

#endif // DISPLAY_MANAGER_H
