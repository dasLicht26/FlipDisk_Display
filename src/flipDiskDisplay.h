#ifndef FLIPDISKDISPLAY_H
#define FLIPDISKDISPLAY_H

#include <Arduino.h>
#include <SPI.h>
#include "config.h"

class FlipDiskDisplay {
public:
    FlipDiskDisplay();
    void begin();
    void clearDisplay();
    void setPixel(uint8_t x, uint8_t y, bool state);
    void updateDisplay();
    void setAllPixels(bool state);

    // Make the new_display array accessible to WebServer
    uint16_t new_display[DISPLAY_WIDTH];

private:
    uint16_t current_display[DISPLAY_WIDTH];

    void clearRegisters();
    void loadSingleDot(uint8_t x, uint8_t y);
    void setFlip(bool dir);
    void sendPulse(int pulse_length);
};

#endif  // FLIPDISKDISPLAY_H
