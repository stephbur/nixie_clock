#ifndef BUTTONS_H
#define BUTTONS_H

void initButtons();
void updateButtons();              // new: must be called each loop
bool isButtonPressed(int buttonPin);

#endif