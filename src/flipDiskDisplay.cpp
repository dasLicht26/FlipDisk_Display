#include "flipDiskDisplay.h"

FlipDiskDisplay::FlipDiskDisplay() {
    // Constructor
}

void FlipDiskDisplay::begin() {
    // Define outputs
    pinMode(PIN_CS_ROW, OUTPUT);
    pinMode(PIN_CS_COL, OUTPUT);
    pinMode(PIN_RST_ROW, OUTPUT);
    pinMode(PIN_RST_COL, OUTPUT);
    pinMode(PIN_SET_RESET, OUTPUT);
    pinMode(PIN_SET_RESET_SAVE, OUTPUT);
    pinMode(PIN_PULSE, OUTPUT);

    // Initial output states
    digitalWrite(PIN_SET_RESET_SAVE, LOW);
    digitalWrite(PIN_SET_RESET, LOW);
    digitalWrite(PIN_PULSE, LOW);

    // Clear shift registers
    clearRegisters();

    // Start SPI communication
    SPI.begin();
}

void FlipDiskDisplay::clearRegisters() {
    // Resets shift registers
    digitalWrite(PIN_RST_ROW, LOW);
    digitalWrite(PIN_RST_COL, LOW);
    delayMicroseconds(1);
    digitalWrite(PIN_RST_ROW, HIGH);
    digitalWrite(PIN_RST_COL, HIGH);
}

void FlipDiskDisplay::loadSingleDot(uint8_t x, uint8_t y) {
    // Pre-load shift registers with zeros and set one bit
    clearRegisters();

    // Adjust for unused outputs
    if (x >= 30) {
        x += 2;
    }

    // Start SPI transaction
    SPI.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE0));

    // Coordinate X
    digitalWrite(PIN_CS_COL, LOW);
    for (int b = 7; b >= 0; b--) {
        if ((x / 8) == b) {
            SPI.transfer(0x01 << (x % 8));
        } else {
            SPI.transfer(0x00);
        }
    }
    digitalWrite(PIN_CS_COL, HIGH);

    // Coordinate Y
    digitalWrite(PIN_CS_ROW, LOW);
    if (x >= 32) {
        y += 16;
    }
    for (int b = 3; b >= 0; b--) {
        if ((y / 8) == b) {
            SPI.transfer(0x01 << (y % 8));
        } else {
            SPI.transfer(0x00);
        }
    }
    digitalWrite(PIN_CS_ROW, HIGH);

    // End SPI transaction
    SPI.endTransaction();
}

void FlipDiskDisplay::setFlip(bool dir) {
    // Set or reset mode
    digitalWrite(PIN_SET_RESET, dir);
    delayMicroseconds(4);
    digitalWrite(PIN_SET_RESET_SAVE, HIGH);
    delayMicroseconds(4);
    digitalWrite(PIN_SET_RESET_SAVE, LOW);
}

void FlipDiskDisplay::sendPulse(int pulse_length) {
    // Send a pulse to the shift registers
    digitalWrite(PIN_PULSE, HIGH);
    delayMicroseconds(pulse_length);
    digitalWrite(PIN_PULSE, LOW);
}

void FlipDiskDisplay::setPixel(uint8_t x, uint8_t y, bool state) {
    // Set a pixel in new_display
    if (state) {
        new_display[x] |= (1 << y);
    } else {
        new_display[x] &= ~(1 << y);
    }
}

void FlipDiskDisplay::updateDisplay() {
    for (int x = 0; x < DISPLAY_WIDTH; x++) {
        for (int y = 0; y < DISPLAY_HEIGHT; y++) {
            bool currentState = current_display[x] & (1 << y);
            bool newState = new_display[x] & (1 << y);

            if (currentState != newState) {
                // Update pixel
                loadSingleDot(x, y);
                setFlip(newState ? SET : RESET);
                delay(2);
                sendPulse(PULSE_LENGTH_US);

                // Update current_display
                if (newState) {
                    current_display[x] |= (1 << y);
                } else {
                    current_display[x] &= ~(1 << y);
                }
            }
        }
    }
}

void FlipDiskDisplay::clearDisplay() {
    // Clear the display
    memset(current_display, 0, sizeof(current_display));
    memset(new_display, 0, sizeof(new_display));
    setAllPixels(RESET);
}

void FlipDiskDisplay::setAllPixels(bool state) {
    for (int x = 0; x < DISPLAY_WIDTH; x++) {
        for (int y = 0; y < DISPLAY_HEIGHT; y++) {
            loadSingleDot(x, y);
            setFlip(state);
            delay(4);
            sendPulse(PULSE_LENGTH_US);
        }
    }
}
