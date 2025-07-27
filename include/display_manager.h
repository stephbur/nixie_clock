#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <IPAddress.h>

class NixieDisplay;

void displayManagerInit(NixieDisplay& display);
void updateDisplayManager();
void triggerTempHumiDisplay();         // externally triggered (e.g. by time or button)
void triggerSlotMachine();             // externally triggered (e.g. by time or button)
void showIpChunks(const IPAddress& ip); // display IP in two chunks

#endif // DISPLAY_MANAGER_H
