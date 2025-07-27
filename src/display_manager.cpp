#include "display_manager.h"
#include "sensors.h"
#include "nixiedisplay.h"
#include "mqtt.h"
#include "ntp.h"
#include "nixiewebserver.h"

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

    mqttPublishStatus(data);

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

void showIpChunks(const IPAddress& ip) {
    if (!nixie) return;
    char ipStr[13] = {0};
    snprintf(ipStr, sizeof(ipStr), "%03d%03d%03d%03d", ip[0], ip[1], ip[2], ip[3]);

    auto display6digits = [&](const char* digits) {
        nixie->disableAllSegments();
        nixie->enableSegment(hourTens[digits[0] - '0']);
        nixie->enableSegment(hourUnits[digits[1] - '0']);
        nixie->enableSegment(minuteTens[digits[2] - '0']);
        nixie->enableSegment(minuteUnits[digits[3] - '0']);
        nixie->enableSegment(secondTens[digits[4] - '0']);
        nixie->enableSegment(secondUnits[digits[5] - '0']);
        nixie->updateDisplay();
    };

    display6digits(ipStr);
    delay(2000);
    display6digits(ipStr + 6);
    delay(2000);
}

void updateDisplayManager() {
    mqttLoop();
    if (isMqttDisplayOverrideActive()) return;

    if (!nixie) return;

    String currentTime = getFormattedTime();
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
    // clockUpdateEnabled is set by the debug interface on the web server.
    if (overrideState == OverrideState::None && clockUpdateEnabled) {
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

