#include "buttons.h"
#include "config.h"
#include <Arduino.h>

#define DEBOUNCE_DELAY 50

// Order matters: must match ButtonID enum
static const int buttonPins[NUM_BUTTONS] = {
    BUTTON1_PIN,
    BUTTON2_PIN,
    BUTTON3_PIN
};

static bool buttonState[NUM_BUTTONS];            // debounced state
static bool lastReading[NUM_BUTTONS];            // last raw read
static bool justPressed[NUM_BUTTONS];            // just pressed (edge detect)
static bool lastButtonState[NUM_BUTTONS];        // previous debounced state
static unsigned long lastDebounceTime[NUM_BUTTONS];

void initButtons() {
    for (int i = 0; i < NUM_BUTTONS; ++i) {
        pinMode(buttonPins[i], INPUT_PULLUP);
        bool reading = digitalRead(buttonPins[i]);
        buttonState[i] = reading;
        lastReading[i] = reading;
        lastButtonState[i] = reading;
        justPressed[i] = false;
        lastDebounceTime[i] = 0;
    }
}

void updateButtons() {
    unsigned long now = millis();
    for (int i = 0; i < NUM_BUTTONS; ++i) {
        bool reading = digitalRead(buttonPins[i]);

        if (reading != lastReading[i]) {
            lastDebounceTime[i] = now;
        }

        if ((now - lastDebounceTime[i]) > DEBOUNCE_DELAY) {
            if (reading != buttonState[i]) {
                buttonState[i] = reading;
            }
        }

        justPressed[i] = (lastButtonState[i] == HIGH && buttonState[i] == LOW);
        lastButtonState[i] = buttonState[i];
        lastReading[i] = reading;
    }
}

bool isButtonPressed(ButtonID button) {
    if (button < NUM_BUTTONS) {
        return buttonState[button] == LOW;  // active low
    }
    return false;
}

bool wasButtonJustPressed(ButtonID button) {
    if (button < NUM_BUTTONS) {
        return justPressed[button];
    }
    return false;
}