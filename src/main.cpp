#include <Arduino.h>
#include "FlipDiskDisplay.h"
#include "MyWebServer.h"

FlipDiskDisplay display;
MyWebServer webServer(&display);

void setup() {
    Serial.begin(115200);
    display.begin();
    webServer.begin();
    display.clearDisplay();
}

void loop() {
    webServer.handleClient();
}
