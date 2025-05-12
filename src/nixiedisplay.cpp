#include "nixiedisplay.h"
#include "config.h"  // Uses SR_CLK_PIN, SR_DIN_PIN, SR_LE_PIN, HV_ENABLE_PIN
#include <Arduino.h>

NixieDisplay::NixieDisplay() {
  // Initialize the frame to all zeros (all segments off)
  for (int i = 0; i < 8; i++) {
    _frame[i] = 0x00;
  }
}

void NixieDisplay::begin() {
  pinMode(SR_LE_PIN, OUTPUT);
  pinMode(SR_DIN_PIN, OUTPUT);
  pinMode(SR_CLK_PIN, OUTPUT);
  pinMode(HV_ENABLE_PIN, OUTPUT);
  
  // The PCB ties _BL and _POL to 5V; enable HV by writing HIGH on HV_ENABLE_PIN.
  digitalWrite(HV_ENABLE_PIN, HIGH);
  
  disableAllSegments();
  updateDisplay(); // Write 64 zeros to the shift registers.
}

void NixieDisplay::enableSegment(byte segment) {
  // In your working code, enabling a segment means setting the bit.
  byte f = 7 - (segment / 8);
  byte b = segment % 8;
  _frame[f] |= (1 << b);
}

void NixieDisplay::disableSegments(const byte segments[], int count) {
  for (int i = 0; i < count; i++) {
    byte seg = segments[i];
    disableSegment(seg);
  }
}

void NixieDisplay::disableAllSegments() {
  // Set the entire frame to zero (all segments off)
  for (int i = 0; i < 8; i++) {
    _frame[i] = 0x00;
  }
}

void NixieDisplay::disableSegment(byte segment) {
  // Clear the corresponding bit.
  byte f = 7 - (segment / 8);
  byte b = segment % 8;
  _frame[f] &= ~(1 << b);
}

void NixieDisplay::updateDisplay() {
  digitalWrite(SR_LE_PIN, LOW);
  for (int i = 0; i < 8; i++) {
    // Instead of Arduino's fast shiftOut(), use our slower version.
    shiftOut(SR_DIN_PIN, SR_CLK_PIN, MSBFIRST, _frame[i]);
  }
  digitalWrite(SR_LE_PIN, HIGH);
}

void NixieDisplay::runSlotMachine() {
  // Run a slot-machine effect; the segment arrays hourTens, hourUnits, etc. must
  // be defined globally (or in a Globals.h).
  for (int j = 0; j < 5; j++) {
    for (int i = 0; i < 10; i++) {
      disableAllSegments();
      enableSegment(hourTens[i]);
      enableSegment(hourUnits[i]);
      enableSegment(minuteTens[i]);
      enableSegment(minuteUnits[i]);
      enableSegment(secondTens[i]);
      enableSegment(secondUnits[i]);
      updateDisplay();
      delay(120);  // Keep the display on for 100 ms per frame (this remains unchanged)
    }
  }
}

// does not produce desired effect yet, maybe too fast?
void NixieDisplay::runSlotMachineWavy() {

    const byte* maps[6] = { hourTens, hourUnits, minuteTens, minuteUnits, secondTens, secondUnits };
    for (int cycle = 0; cycle < 5; ++cycle) {
        for (int i = 0; i < 10; ++i) {
            disableAllSegments();
            for (int pos = 0; pos < 6; ++pos) {
                int d = (i + pos) % 10;
                enableSegment(maps[pos][d]);
            }
            updateDisplay();
            delay(100);
        }
    }
}


// The test function that converts a 6-digit number (e.g. HHMMSS) to display.
// The mapping is identical to your working project.
void NixieDisplay::showNumber(uint32_t number) {
  char buf[7];
  snprintf(buf, sizeof(buf), "%06lu", number);
  
  disableAllSegments();
  int hour   = (buf[0]-'0')*10 + (buf[1]-'0');
  int minute = (buf[2]-'0')*10 + (buf[3]-'0');
  int second = (buf[4]-'0')*10 + (buf[5]-'0');
  
  enableSegment(hourTens[hour / 10]);
  enableSegment(hourUnits[hour % 10]);
  
  enableSegment(minuteTens[minute / 10]);
  enableSegment(minuteUnits[minute % 10]);
  
  enableSegment(secondTens[second / 10]);
  enableSegment(secondUnits[second % 10]);
  
  updateDisplay();
}

