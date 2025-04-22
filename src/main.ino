#include "core/MainController.h"
#include "core/Debug.h"

MainController controller;
unsigned long lastLoopTime = 0;

void setup() {
    Debug::begin(115200);
    Debug::log(Debug::INFO, "Starting initialization...");
    
    try {
        controller.begin();
        Debug::log(Debug::INFO, "Controller initialized successfully");
    } catch (const std::exception& e) {
        Debug::logf(Debug::ERROR, "Controller initialization failed: %s", e.what());
        while(1) {
            delay(1000);
        }
    }

    Debug::log(Debug::INFO, "Setup completed");
}

void loop() {
    unsigned long currentTime = millis();
    unsigned long loopDuration = currentTime - lastLoopTime;
    
    // Log if loop takes too long
    if (loopDuration > 100) { // 100ms threshold
        Debug::logf(Debug::DEBUG, "Long loop detected: %lu ms", loopDuration);
    }

    try {
        controller.update();
    } catch (const std::exception& e) {
        Debug::logf(Debug::ERROR, "Loop error: %s", e.what());
        delay(1000);
    }

    lastLoopTime = currentTime;
}
