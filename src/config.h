#ifndef CONFIG_H
#define CONFIG_H

// Display parameters
#define DISPLAY_WIDTH 30    // Display width
#define DISPLAY_HEIGHT 16   // Display height

// SPI pins for sending data into shift registers
#define SPI_SPEED 1000000   // SPI speed: 1 MHz
#define PIN_CS_ROW 26       // Chip Select pin for row shift registers
#define PIN_CS_COL 33       // Chip Select pin for column shift registers
#define PIN_RST_ROW 17      // Reset pin for row shift registers
#define PIN_RST_COL 16      // Reset pin for column shift registers

// Pins used for flipping dots
#define PIN_SET_RESET_SAVE 15  // Save the state of the dot
#define PIN_SET_RESET 32
#define PIN_PULSE 12

// Pins for LED control
#define PIN_LED_OE 27  // PWM PIN
#define PIN_LED_STR 22

// Fixed parameters
#define PULSE_LENGTH_US 1000  // Pulse length in microseconds
#define SET 1                 // HIGH
#define RESET 0               // LOW

// WLAN credentials
//#define WIFI_SSID "BND-area-monitoring-NWO-01917"
#define WIFI_SSID "wireless"
//#define WIFI_PASSWORD "Johapanula220916"
#define WIFI_PASSWORD "12345678900"

#endif  // CONFIG_H
