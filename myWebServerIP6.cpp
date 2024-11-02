#include "myWebServer.h"

MyWebServer::MyWebServer(FlipDiskDisplay* display) : server(80) {
    this->display = display;
}

void MyWebServer::begin() {
    // Connect to Wi-Fi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("Connected to Wi-Fi");

    // IPv6-Unterstützung aktivieren
    if (!WiFi.enableIpV6()) {
        Serial.println("Failed to enable IPv6");
    } else {
        Serial.println("IPv6 enabled");
    }


    delay(500);
    // Warten, bis eine IPv6-Adresse zugewiesen wurde
    while (WiFi.localIPv6().toString() == "0000:0000:0000:0000:0000:0000:0000:0000") {
        delay(500);
        Serial.print(".");
    }

    Serial.print("IPv6 address: ");
    Serial.println(WiFi.localIPv6());

    Serial.print("IPv4 address: ");
    Serial.println(WiFi.localIP());

    // Set up web server routes
    server.on("/", HTTP_GET, std::bind(&MyWebServer::handleRoot, this, std::placeholders::_1));
    server.on("/setPixel", HTTP_POST, std::bind(&MyWebServer::handleSetPixel, this, std::placeholders::_1));

    // Start the server
    server.begin();
    Serial.println("HTTP server started");
}

void MyWebServer::handleRoot(AsyncWebServerRequest *request) {
    String html = R"rawliteral(
    <!-- Ihr HTML-Code hier -->
    )rawliteral";

    request->send(200, "text/html", html);
}

void MyWebServer::handleSetPixel(AsyncWebServerRequest *request) {
    if (request->hasParam("plain", true)) {
        String json = request->getParam("plain", true)->value();
        JsonDocument doc;

        DeserializationError error = deserializeJson(doc, json);

        if (error) {
            request->send(400, "text/plain", "Invalid JSON");
            return;
        }

        // Clear the new_display array
        memset(display->new_display, 0, sizeof(display->new_display));

        for (int x = 0; x < DISPLAY_WIDTH; x++) {
            for (int y = 0; y < DISPLAY_HEIGHT; y++) {
                if (doc[x][y]) {
                    display->setPixel(x, y, true);
                } else {
                    display->setPixel(x, y, false);
                }
            }
        }

        display->updateDisplay();
        request->send(200, "text/plain", "OK");
    } else {
        request->send(400, "text/plain", "Bad Request");
    }
}
