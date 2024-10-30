#ifndef MYWEBSERVER_H
#define MYWEBSERVER_H

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "FlipDiskDisplay.h"

class MyWebServer {
public:
    MyWebServer(FlipDiskDisplay* display);
    void begin();
private:
    FlipDiskDisplay* display;
    AsyncWebServer server;
    void handleRoot(AsyncWebServerRequest *request);
    void handleSetPixel(AsyncWebServerRequest *request);
};

#endif  // MYWEBSERVER_H
