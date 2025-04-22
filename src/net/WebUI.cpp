#include "WebUI.h"

WebUI::WebUI() : server(80) {
    // Initialize the WebServer instance
}

void WebUI::begin() {
    WiFi.softAP("GlitchGlimmer", "glimmer123");

    server.on("/", [this]() { handleRoot(); });
    server.onNotFound([this]() { handleNotFound(); });

    server.begin();
}

void WebUI::update() {
    server.handleClient();
}

void WebUI::handleRoot() {
    String html = R"rawliteral(
      <!DOCTYPE html>
      <html>
      <head><title>GlitchGlimmer Control</title></head>
      <body style="font-family: sans-serif;">
        <h2>🎛 GlitchGlimmer Controller</h2>
        <p>Control UI coming soon!</p>
      </body>
      </html>
    )rawliteral";

    server.send(200, "text/html", html);
}

void WebUI::handleNotFound() {
    server.send(404, "text/plain", "Not Found");
}
