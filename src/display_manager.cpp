#include "display_manager.h"
#include "sensors.h"
#include "nixiedisplay.h"

static NixieDisplay* nixie = nullptr;
static int lastTempHumiDisplayMinute = -1;
static int lastSlotMachineMinute = -1;

enum class OverrideState {
    None,
    TempHumiDisplay,
    PressureDisplay,
    SlotMachine
};

static OverrideState overrideState = OverrideState::None;
static unsigned long overrideStart = 0;

void displayManagerInit(NixieDisplay& display) {
    nixie = &display;
}

void triggerTempHumiDisplay() {
    if (!nixie) return;
    overrideStart = millis();
    overrideState = OverrideState::TempHumiDisplay;

    SensorData data = readSensors();
    int temp = round(data.temperatureAHT);
    int humi = round(data.humidity);

    nixie->disableAllSegments();
    nixie->enableSegment(hourTens[temp / 10]);
    nixie->enableSegment(hourUnits[temp % 10]);
    nixie->enableSegment(secondTens[humi / 10]);
    nixie->enableSegment(secondUnits[humi % 10]);
    nixie->updateDisplay();
}

void triggerPressureDisplay() {
    if (!nixie) return;
    overrideState = OverrideState::PressureDisplay;

    SensorData data = readSensors();
    int pres = round(data.pressure); // e.g. 1013
    int hi = pres / 100;
    int lo = pres % 100;

    nixie->disableAllSegments();
    nixie->enableSegment(minuteUnits[hi % 10]);
    nixie->enableSegment(secondTens[lo / 10]);
    nixie->enableSegment(secondUnits[lo % 10]);
    nixie->updateDisplay();
}

void triggerSlotMachine() {
    if (!nixie || overrideState == OverrideState::SlotMachine) return;

    overrideStart = millis();
    overrideState = OverrideState::SlotMachine;

    nixie->runSlotMachine(); 
    overrideState = OverrideState::None;
}

void updateDisplayManager(const String& currentTime) {
    if (!nixie) return;

    int h, m, s;
    if (sscanf(currentTime.c_str(), "%d:%d:%d", &h, &m, &s) != 3) return;

    unsigned long now = millis();

    // Scheduled sensor display every 5 minutes + 1 (e.g. 06, 11, ...)
    if (m % 5 == 1 && s == 0 && m != lastTempHumiDisplayMinute) {
        lastTempHumiDisplayMinute = m;
        triggerTempHumiDisplay();
    }
    // Override state transitions
    // if it is displaying TempHumi and 3 seconds have passed, switch to PressureDisplay
    if (overrideState == OverrideState::TempHumiDisplay && now - overrideStart >= 3000) {
        triggerPressureDisplay();
    // if it is displaying PressureDisplay and 6 seconds have passed, disable override
    } else if (overrideState == OverrideState::PressureDisplay && now - overrideStart >= 6000) {
        overrideState = OverrideState::None;
    }

    // Scheduled slot machine effect
    if (m % 5 == 0 && s == 0 && m != lastSlotMachineMinute) {
        lastSlotMachineMinute = m;
        triggerSlotMachine();
    }

    // Default time display (only when no override is active)
    if (overrideState == OverrideState::None) {
        uint32_t compactTime = h * 10000 + m * 100 + s;
        nixie->showNumber(compactTime);

        if (s % 2 == 0) {
            nixie->disableSegment(leftDot);
            nixie->disableSegment(rightDot);
        } else {
            nixie->enableSegment(leftDot);
            nixie->enableSegment(rightDot);
        }

        nixie->updateDisplay();
    }
}

