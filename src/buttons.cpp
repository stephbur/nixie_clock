#include "buttons.h"
#include "config.h"
#include <Arduino.h>

#define DEBOUNCE_DELAY 50

static const int buttonPins[]    = { BUTTON1_PIN, BUTTON2_PIN, BUTTON3_PIN };
static bool      buttonState[3];
static bool      lastReading[3];
static unsigned long lastDebounceTime[3];

void initButtons() {
  for (int i = 0; i < 3; ++i) {
    pinMode(buttonPins[i], INPUT_PULLUP);
    lastReading[i]      = digitalRead(buttonPins[i]);
    buttonState[i]      = lastReading[i];
    lastDebounceTime[i] = 0;
  }
}

void updateButtons() {
  unsigned long now = millis();
  for (int i = 0; i < 3; ++i) {
    bool reading = digitalRead(buttonPins[i]);
    if (reading != lastReading[i]) {
      lastDebounceTime[i] = now;
    }
    if ((now - lastDebounceTime[i]) > DEBOUNCE_DELAY) {
      // stable for long enough?
      buttonState[i] = reading;
    }
    lastReading[i] = reading;
  }
}

bool isButtonPressed(int buttonPin) {
  for (int i = 0; i < 3; ++i) {
    if (buttonPins[i] == buttonPin) {
      // active‐low
      return buttonState[i] == LOW;
    }
  }
  return false;
}
