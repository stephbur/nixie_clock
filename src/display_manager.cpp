#include "display_manager.h"
#include "sensors.h"
#include "nixiedisplay.h"

static NixieDisplay* nixie = nullptr;
static int lastSensorDisplayMinute = -1;
static int lastSlotMachineMinute = -1;

static unsigned long overrideStart = 0;
static int overridePhase = 0; // 0 = none, 1 = sensor display, 2 = pressure display
static bool runningSlotMachine = false;

void displayManagerInit(NixieDisplay& display) {
    nixie = &display;
}

void triggerSensorDisplay() {
    if (!nixie) return;
    overrideStart = millis();
    overridePhase = 1;

    SensorData data = readSensors();
    int temp = round(data.temperatureAHT);  // e.g. 23
    int humi = round(data.humidity);        // e.g. 56

    nixie->disableAllSegments();

    nixie->enableSegment(hourTens[temp/10]);
    nixie->enableSegment(hourUnits[temp%10]);
    nixie->enableSegment(secondTens[humi/10]);
    nixie->enableSegment(secondUnits[humi%10]);
    nixie->updateDisplay();
}


void triggerSlotMachine() {
    if (!nixie || runningSlotMachine) return;
    runningSlotMachine = true;
    nixie->runSlotMachine();
    runningSlotMachine = false;
}

void updateDisplayManager(const String& currentTime) {
    if (!nixie) return;

    int h, m, s;
    if (sscanf(currentTime.c_str(), "%d:%d:%d", &h, &m, &s) != 3) return;

    unsigned long now = millis();

    // Scheduled sensor display every 5 minutes + 1 (e.g. 06, 11, ...)
    if (m % 5 == 1 && s == 0 && m != lastSensorDisplayMinute) {
        lastSensorDisplayMinute = m;
        triggerSensorDisplay();
    }

    if (overridePhase == 1 && now - overrideStart >= 3000) {
        overridePhase = 2;
        SensorData data = readSensors();
        int pres   = round(data.pressure);     // e.g. 1013
        int hi     = pres / 100;               // e.g. 10
        int lo     = pres % 100;               // e.g. 13

        nixie->disableAllSegments();
        nixie->enableSegment(minuteUnits[hi%10]);
        nixie->enableSegment(secondTens[lo/10]);
        nixie->enableSegment(secondUnits[lo%10]);
        nixie->updateDisplay();

    } else if (overridePhase == 2 && now - overrideStart >= 6000) {
            overridePhase = 0;
    }

    // Scheduled slot machine effect
    if (m % 5 == 0 && s == 0 && m != lastSlotMachineMinute) {
        lastSlotMachineMinute = m;
        triggerSlotMachine();
    }
}

bool isDisplayOverrideActive() {
    return overridePhase != 0 || runningSlotMachine;
}
