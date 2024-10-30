// myWebServer.h

#ifndef MYWEBSERVER_H
#define MYWEBSERVER_H

#include <WiFi.h>
#include "FlipDiskDisplay.h"

class MyWebServer {
public:
    MyWebServer(FlipDiskDisplay* display);
    void begin();
    void handleClient();

private:
    FlipDiskDisplay* display;
    int server_fd;

    void startServer();
    void handleRoot(int client_fd);
    void handleSetPixel(int client_fd, String &request);
    void handleNotFound(int client_fd);
    void sendResponse(int client_fd, String status, String contentType, String content);
};

#endif
