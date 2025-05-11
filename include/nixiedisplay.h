#ifndef NIXIEDISPLAY_H
#define NIXIEDISPLAY_H

#include <Arduino.h>

// Global segment definitions (matching your PCB wiring)
const byte leftDot       = 11;
const byte rightDot      = 53;
const byte hourTens[]    = {22, 31, 30, 29, 28, 27, 26, 25, 24, 23};
const byte hourUnits[]   = {12, 21, 20, 19, 18, 17, 16, 15, 14, 13};
const byte minuteTens[]  = {1, 10, 9, 8, 7, 6, 5, 4, 3, 2};
const byte minuteUnits[] = {54, 63, 62, 61, 60, 59, 58, 57, 56, 55};
const byte secondTens[]  = {43, 52, 51, 50, 49, 48, 47, 46, 45, 44};
const byte secondUnits[] = {33, 42, 41, 40, 39, 38, 37, 36, 35, 34};

class NixieDisplay {
  public:
    NixieDisplay();
    void begin();
    void enableSegment(byte segment);
    void disableSegments(const byte segments[], int count);
    void disableAllSegments();
    void disableSegment(byte segment);
    void updateDisplay();
    void runSlotMachine();
    // Test function: display a 6-digit number (e.g. HHMMSS)
    void showNumber(uint32_t number);
    // Test function: force display of digit "5" on all tubes.
    void showAll5();
  private:
    // Frame of 8 bytes (64 bits) for the HV5622 chain.
    void slowShiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, byte value);

    // In your working project, a 0 means OFF and a 1 means ON.
    byte _frame[8];
};

#endif // NIXIEDISPLAY_H
