#include "buttons.h"
#include "config.h"
#include <Arduino.h>

void initButtons() {
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(BUTTON3_PIN, INPUT_PULLUP);
}

bool isButtonPressed(int buttonPin) {
  // Return true if the button is pressed (active low)
  return digitalRead(buttonPin) == LOW;
}
