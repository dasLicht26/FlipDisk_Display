#include <Arduino.h>
#include <SPI.h>

// SPI pins for sending data into shift registers
#define SPI_SPEED 1000000  // 0.1 Mhz (Es geht auch mehr, aber dann kann ich es nicht mehr lesen)
#define PIN_CS_ROW 26 //26 - Chip Select pin for row shift registers
#define PIN_CS_COL 33 //33 - Chip Select pin for column shift registers
#define PIN_RST_ROW 17 //17 - Reset pin for row shift registers
#define PIN_RST_COL 16 //16 - Reset pin for column shift registers



// Pins used for flipping dots
#define PIN_SET_RESET_SAVE 15 //15 - Save the state of the dot
#define PIN_SET_RESET 32 //32
#define PIN_PULSE 12 //12

// Pins for LED control
#define PIN_LED_OE 27 // PWM PIN
#define PIN_LED_STR 22

// Fixed parameters
#define PULSE_LENGTH_US 3000  // May be adjusted according to voltage used
#define SET 1  // HIGH
#define RESET 0  // LOW

// Display params
#define DISPLAY_WIDTH 30  // Only 1 display connected for now
#define DISPLAY_HEIGHT 16

uint16_t current_display[DISPLAY_WIDTH];
uint16_t new_display[DISPLAY_WIDTH];

void clear_registers() {
  // Resets shift registers
  digitalWrite(PIN_RST_ROW, LOW);
  digitalWrite(PIN_RST_COL, LOW);
  delayMicroseconds(1);
  digitalWrite(PIN_RST_ROW, HIGH);
  digitalWrite(PIN_RST_COL, HIGH);
}


void load_single_dot(uint8_t x, uint8_t y){
  // This method safely pre-loads all SR with zeros and only 1 bit set at the
  // requested x and y position.

  // Currently this method is implemented for
  // 2 horizontally daisy chained 30x16 displays (2x1)
  //  ______________________________    ______________________________
  // |                              |->|                              |
  // |         DISPLAY 1            |->|         DISPLAY 2            |
  // |         (30 x 16)            |->|         (30 x 16)            |
  // |                              |->|                              |
  // |______________________________|->|______________________________|
  //
  // Any other arrangements "3x1" displays or "2x2" or "1x2" are possible,
  // but modifications have to be made in the order of bytes of data
  // sent to the registers and number of bytes sent to accomodate for each
  // individual setup and how shift registers are connected
  // (vertically vs horizontally). 25 dots width displays can be used as well.

  // Clear shift register data, but if it doesn't work it's not a problem
  // as we send bytes to all SR of the two controllers.
  clear_registers();

  // Insert 2 dummy bits for unused output 31 and 32 of shift registers
  // in the case of a 30 dots wide display with a controller of 4 SR (32 bits)
  // May have to be adjusted if using a 25 dots wide display with 4 SR.
  if(x >= 30) {
    x+=2;
  }

  // Initialise SPI communication
  SPI.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE0));

  // COORDINATE X :
  digitalWrite (PIN_CS_COL, LOW);  // Slave Select Columns shift registers

  for(int b=7; b>=0; b--){
    // Transfer as many bytes as number of column SR on all controllers (8)
    if((int) (x/8) == b){
      // Transfer a 1 into the correct position of correct SR
      SPI.transfer(0x01 << (x%8));
    } else {
      // Transfer 0s to other SR to blank any remaining other data
      SPI.transfer(0x00);
    }
  }

  // End Slave Select Columns
  digitalWrite (PIN_CS_COL, HIGH);


  // COORDINATE Y :
  digitalWrite (PIN_CS_ROW, LOW);  // Slave Select Rows shift registers

  if(x>=32) {
    // Targeting display 2, so we increase y by 16 to address its SR
    y += 16;
  }

  for(int b=3; b>=0; b--){
    // Transfer as many bytes as number of row SR on all controllers (4)
    if((int) (y/8) == b){
      // Transfer a 1 into the correct position of correct SR
      SPI.transfer(0x01 << (y%8));
    } else {
      // Transfer 0s to other SR to blank any remaining other data
      SPI.transfer(0x00);
    }
  }

  digitalWrite (PIN_CS_ROW, HIGH);  // End Slave Select Rows


  SPI.endTransaction();  // End SPI

}

void setFlip(bool dir) {
  // SET or RESET mode
  digitalWrite(PIN_SET_RESET, dir);
  delayMicroseconds(4);
  digitalWrite(PIN_SET_RESET_SAVE, HIGH);
  delayMicroseconds(4);
  digitalWrite(PIN_SET_RESET_SAVE, LOW);
}

void sendPulse(int pulse_length) {
  // Send a pulse to the shift registers
  digitalWrite(PIN_PULSE, HIGH);
  delayMicroseconds(pulse_length);
  digitalWrite(PIN_PULSE, LOW);
}


void setup() {


  // Defining outputs
  pinMode(PIN_CS_ROW, OUTPUT);
  pinMode(PIN_CS_COL, OUTPUT);
  pinMode(PIN_RST_ROW, OUTPUT);
  pinMode(PIN_RST_COL, OUTPUT);
  pinMode(PIN_SET_RESET, OUTPUT);
  pinMode(PIN_SET_RESET_SAVE, OUTPUT);
  pinMode(PIN_PULSE, OUTPUT);

  // Initial outputs state
  digitalWrite(PIN_SET_RESET_SAVE, LOW);
  digitalWrite(PIN_SET_RESET, LOW);
  digitalWrite(PIN_PULSE, LOW);

  // Clear Shift Registers
  clear_registers();

  SPI.begin();

}


void loop() {
  for(int x=0; x<DISPLAY_WIDTH; x++){
    for(int y=0; y<DISPLAY_HEIGHT; y++){
      load_single_dot(x, y);
      setFlip(RESET);
      delay(1);
      sendPulse(400);
    }
  }
  delay(2000);
  for(int x=0; x<DISPLAY_WIDTH; x++){
    for(int y=0; y<DISPLAY_HEIGHT; y++){
      load_single_dot(x, y);
      setFlip(SET);
      delay(1);
      sendPulse(400);
    }
  }
delay(2000);


}

