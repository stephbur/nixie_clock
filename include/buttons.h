#ifndef BUTTONS_H
#define BUTTONS_H

enum ButtonID {
    BUTTON_1 = 0,
    BUTTON_2,
    BUTTON_3,
    NUM_BUTTONS
};

void initButtons();
void updateButtons();

bool isButtonPressed(ButtonID button);
bool wasButtonJustPressed(ButtonID button);

#endif