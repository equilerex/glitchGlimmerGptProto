#include "core/MainController.h"
#include "core/Debug.h"
#include <esp_task_wdt.h>

MainController controller;
unsigned long lastLoopTime = 0;
unsigned long lastMemCheckTime = 0;
int consecutiveErrors = 0;
const int ERROR_THRESHOLD_FOR_RESTART = 5;

void setup() {
    // Initialize debug system first for logging
    Debug::begin(115200);
    Debug::log(Debug::INFO, "Starting initialization...");
    
    // Setup watchdog for crash detection
    esp_task_wdt_init(10, true); // 10 second timeout
    esp_task_wdt_add(NULL); // Register current thread with WDT
    
    // Log memory status at startup
    size_t freeHeap = ESP.getFreeHeap();
    Debug::logf(Debug::INFO, "Initial free heap: %u bytes", freeHeap);
    
    try {
        controller.begin();
        Debug::log(Debug::INFO, "Controller initialized successfully");
    } catch (const std::exception& e) {
        Debug::logf(Debug::ERROR, "Controller initialization failed: %s", e.what());
        while(1) {
            delay(1000);
            esp_task_wdt_reset(); // Feed watchdog to prevent auto-reset
        }
    } catch (...) {
        Debug::log(Debug::ERROR, "Unknown exception during controller initialization");
        while(1) {
            delay(1000);
            esp_task_wdt_reset(); // Feed watchdog to prevent auto-reset
        }
    }

    Debug::log(Debug::INFO, "Setup completed");
}

void loop() {
    // Reset watchdog to prevent timeout
    esp_task_wdt_reset();
    
    unsigned long currentTime = millis();
    unsigned long loopDuration = currentTime - lastLoopTime;
    
    // Log if loop takes too long
    if (loopDuration > 100) { // 100ms threshold
        Debug::logf(Debug::DEBUG, "Long loop detected: %lu ms", loopDuration);
    }
    
    // Periodically log memory status
    if (currentTime - lastMemCheckTime > 30000) { // Every 30 seconds
        size_t freeHeap = ESP.getFreeHeap();
        Debug::logf(Debug::INFO, "Free heap: %u bytes", freeHeap);
        lastMemCheckTime = currentTime;
    }

    try {
        controller.update();
        // Reset error counter on successful iteration
        consecutiveErrors = 0;
    } catch (const std::exception& e) {
        consecutiveErrors++;
        Debug::logf(Debug::ERROR, "Loop error (%d/%d): %s", 
                   consecutiveErrors, ERROR_THRESHOLD_FOR_RESTART, e.what());
        
        // Give system time to recover
        delay(500);
        
        // If too many consecutive errors, restart the system
        if (consecutiveErrors >= ERROR_THRESHOLD_FOR_RESTART) {
            Debug::log(Debug::ERROR, "Too many consecutive errors, restarting...");
            delay(1000); // Allow time for message to be sent
            ESP.restart();
        }
    } catch (...) {
        consecutiveErrors++;
        Debug::logf(Debug::ERROR, "Unknown loop error (%d/%d)", 
                   consecutiveErrors, ERROR_THRESHOLD_FOR_RESTART);
        
        // Give system time to recover
        delay(500);
        
        // If too many consecutive errors, restart the system
        if (consecutiveErrors >= ERROR_THRESHOLD_FOR_RESTART) {
            Debug::log(Debug::ERROR, "Too many consecutive errors, restarting...");
            delay(1000); // Allow time for message to be sent
            ESP.restart();
        }
    }

    lastLoopTime = currentTime;
}
