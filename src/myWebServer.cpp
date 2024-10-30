#include "myWebServer.h"
#include <WiFi.h>
#include <ArduinoJson.h>
#include <lwip/sockets.h> // Für Low-Level-Socket-Programmierung
#include <lwip/inet.h>
#include <lwip/netdb.h>


MyWebServer::MyWebServer(FlipDiskDisplay* display) {
    this->display = display;
    server_fd = -1;
}

void MyWebServer::begin() {
    // Verbinden mit Wi-Fi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Verbinde mit Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("Mit Wi-Fi verbunden");

    // IPv6-Unterstützung aktivieren
    if (!WiFi.enableIpV6()) {
        Serial.println("IPv6 konnte nicht aktiviert werden");
    } else {
        Serial.println("IPv6 aktiviert");
    }

    // Warten, bis eine IPv6-Adresse zugewiesen wurde
    while (WiFi.localIPv6().toString() == "0000:0000:0000:0000:0000:0000:0000:0000") {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.print("IPv6-Adresse: ");
    Serial.println(WiFi.localIPv6());

    Serial.print("IPv4-Adresse: ");
    Serial.println(WiFi.localIP());

    // Server starten
    startServer();
    Serial.println("HTTP-Server gestartet");
}

void MyWebServer::startServer() {
    // Erstellen eines IPv6-Sockets
    server_fd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd < 0) {
        Serial.println("Fehler beim Erstellen des Sockets");
        return;
    }

    // Socket-Optionen setzen
    int yes = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
        Serial.println("Fehler beim Setzen von SO_REUSEADDR");
        close(server_fd);
        return;
    }

    // Socket an alle IPv6-Adressen binden
    struct sockaddr_in6 server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(8080);
    server_addr.sin6_addr = in6addr_any; // Lauscht auf allen Schnittstellen

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        Serial.println("Fehler beim Binden des Sockets");
        close(server_fd);
        return;
    }

    // Lauschen auf eingehende Verbindungen
    if (listen(server_fd, 5) < 0) {
        Serial.println("Fehler beim Lauschen auf dem Socket");
        close(server_fd);
        return;
    }
}

void MyWebServer::handleClient() {
    if (server_fd < 0) return;

    struct sockaddr_in6 client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Nicht blockierendes accept
    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0) {
        // Kein Client verfügbar
        return;
    }

    // Client-Verbindung bearbeiten
    char buffer[2048];
    int len = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (len > 0) {
        buffer[len] = '\0';
        String request = String(buffer);

        // Anfrage analysieren
        if (request.startsWith("GET / ")) {
            handleRoot(client_fd);
        } else if (request.startsWith("POST /setPixel")) {
            handleSetPixel(client_fd, request);
        } else {
            handleNotFound(client_fd);
        }
    }
    close(client_fd);
}

void MyWebServer::handleRoot(int client_fd) {
    String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
        <title>Paul's FlipDisk Display Control</title>
        <style>
            body {
                font-family: Arial, sans-serif;
                text-align: center;
            }
            #displayCanvas {
                border: 1px solid #000;
                touch-action: none;
                margin-top: 20px;
            }
            #updateButton {
                padding: 10px 20px;
                font-size: 16px;
                margin-top: 15px;
                cursor: pointer;
            }
        </style>
    </head>
    <body>
        <h1>Paul's FlipDisk Display Control</h1>
        <canvas id="displayCanvas" width=")rawliteral" + String(DISPLAY_WIDTH * 20) + R"rawliteral(" height=")rawliteral" + String(DISPLAY_HEIGHT * 20) + R"rawliteral("></canvas>
        <br>
        <button id="updateButton" onclick="sendData()">Zeichnen</button>
        <script>
            // Konfiguration
            const displayWidth = )rawliteral" + String(DISPLAY_WIDTH) + R"rawliteral(;
            const displayHeight = )rawliteral" + String(DISPLAY_HEIGHT) + R"rawliteral(;
            const pixelSize = 20; // Größe jedes Pixels im Canvas

            // Canvas-Einrichtung
            const canvas = document.getElementById('displayCanvas');
            const ctx = canvas.getContext('2d');
            canvas.width = displayWidth * pixelSize;
            canvas.height = displayHeight * pixelSize;

            // Initialisiere Pixel-Datenarray
            let pixelData = [];
            for (let x = 0; x < displayWidth; x++) {
                pixelData[x] = [];
                for (let y = 0; y < displayHeight; y++) {
                    pixelData[x][y] = 0; // Alle Pixel beginnen im 'Aus'-Zustand
                }
            }

            // Zeichne das initiale Gitter
            drawGrid();

            // Funktion zum Zeichnen des gesamten Gitters
            function drawGrid() {
                for (let x = 0; x < displayWidth; x++) {
                    for (let y = 0; y < displayHeight; y++) {
                        drawPixel(x, y, pixelData[x][y]);
                    }
                }
            }

            // Funktion zum Zeichnen eines einzelnen Pixels
            function drawPixel(x, y, state) {
                ctx.fillStyle = state ? 'yellow' : 'black';
                ctx.fillRect(x * pixelSize, y * pixelSize, pixelSize, pixelSize);
                ctx.strokeStyle = 'gray';
                ctx.strokeRect(x * pixelSize, y * pixelSize, pixelSize, pixelSize);
            }

            // Event-Listener für Klicks auf das Canvas
            canvas.addEventListener('click', function(event) {
                const rect = canvas.getBoundingClientRect();
                const x = Math.floor((event.clientX - rect.left) / pixelSize);
                const y = Math.floor((event.clientY - rect.top) / pixelSize);
                if (x >= 0 && x < displayWidth && y >= 0 && y < displayHeight) {
                    pixelData[x][y] = pixelData[x][y] ? 0 : 1; // Schaltet den Pixelzustand um
                    drawPixel(x, y, pixelData[x][y]);
                }
            });

            // Funktion zum Senden der Pixel-Daten an den Server
            function sendData() {
                fetch('/setPixel', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json'
                    },
                    body: JSON.stringify(pixelData)
                })
                .then(response => {
                    if (response.ok) {
                        alert('Display erfolgreich aktualisiert!');
                    } else {
                        alert('Fehler beim Aktualisieren des Displays.');
                    }
                })
                .catch(error => {
                    console.error('Fehler:', error);
                });
            }
        </script>
    </body>
    </html>
    )rawliteral";

    sendResponse(client_fd, "200 OK", "text/html", html);
}

void MyWebServer::handleSetPixel(int client_fd, String &request) {
    // Den Body der Anfrage extrahieren
    int bodyIndex = request.indexOf("\r\n\r\n");
    String json = request.substring(bodyIndex + 4);

    // JSON parsen
    JsonDocument doc; // Größe ggf. anpassen
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
        sendResponse(client_fd, "400 Bad Request", "text/plain", "Ungültiges JSON");
        return;
    }

    // new_display-Array leeren
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
    sendResponse(client_fd, "200 OK", "text/plain", "OK");
}

void MyWebServer::handleNotFound(int client_fd) {
    sendResponse(client_fd, "404 Not Found", "text/plain", "404: Nicht gefunden");
}

void MyWebServer::sendResponse(int client_fd, String status, String contentType, String content) {
    String response = "HTTP/1.1 " + status + "\r\n";
    response += "Content-Type: " + contentType + "\r\n";
    response += "Content-Length: " + String(content.length()) + "\r\n";
    response += "Connection: close\r\n";
    response += "\r\n";
    response += content;

    send(client_fd, response.c_str(), response.length(), 0);
}
