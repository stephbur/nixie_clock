#ifndef BUTTONS_H
#define BUTTONS_H

// Initialize the button pins
void initButtons();

// Check if a button (by its pin) is pressed (assuming active LOW)
bool isButtonPressed(int buttonPin);

#endif // BUTTONS_H
