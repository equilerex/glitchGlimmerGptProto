#ifndef WEBUI_H
#define WEBUI_H

#include <WiFi.h>
#include <WebServer.h>

class WebUI {
public:
    WebUI(); // Declare the constructor
    void begin();
    void update();

private:
    void handleRoot();
    void handleNotFound();

    WebServer server;
};
#endif // WEBUI_H