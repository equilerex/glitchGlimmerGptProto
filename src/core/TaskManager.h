#pragma once
#include <esp_task_wdt.h>
#include <Arduino.h>

// For memory debugging
#define IS_PTR_VALID(ptr) TaskManager::isPointerValid(ptr)
#define LOG_PTR(name, ptr) TaskManager::logPointer(name, ptr, __FILE__, __LINE__)

class TaskManager {
public:
    static void init() {
        esp_task_wdt_init(60, true); // 60 second timeout
        esp_task_wdt_add(NULL);
        lastYield = 0;
        lastMemCheck = 0;
        Serial.println("TaskManager initialized with 60s timeout");
        pinMode(LED_BUILTIN, OUTPUT);
    }

    static void feedWatchdog() {
        esp_task_wdt_reset();
        checkMemory();
    }

    static void yieldIfNeeded() {
        unsigned long now = millis();
        if (now - lastYield > yieldInterval) {
            yield();
            lastYield = now;
            feedWatchdog();
        }
    }
    
    // Check if a pointer is likely valid
    static bool isPointerValid(const void* ptr) {
        if (ptr == nullptr) return false;
        
        // Check if pointer is in valid ESP32 RAM range
        uint32_t addr = reinterpret_cast<uint32_t>(ptr);
        
        // DRAM range typically 0x3FFB0000 to 0x3FFFFFFF
        if (addr >= 0x3FFB0000 && addr <= 0x3FFFFFFF) return true;
        
        // IRAM range typically 0x40000000 to 0x40400000
        if (addr >= 0x40000000 && addr <= 0x40400000) return true;
        
        return false;
    }
    
    // Helper to log pointer information
    static void logPointer(const char* name, const void* ptr, const char* file, int line) {
        if (!isPointerValid(ptr)) {
            Serial.printf("[WARNING] Invalid pointer %s = %p at %s:%d\n", name, ptr, file, line);
        } else {
            // Uncomment for verbose debugging
            // Serial.printf("[DEBUG] Valid pointer %s = %p at %s:%d\n", name, ptr, file, line);
        }
    }

private:
    static void checkMemory() {
        unsigned long now = millis();
        if (now - lastMemCheck > 5000) { // Check every 5 seconds
            size_t freeHeap = ESP.getFreeHeap();
            if (freeHeap < 20000) {
                Serial.printf("Low memory warning: %d bytes free\n", freeHeap);
            }
            lastMemCheck = now;
        }
    }

    static constexpr unsigned long yieldInterval = 5;
    static unsigned long lastYield;
    static unsigned long lastMemCheck;
};

unsigned long TaskManager::lastYield = 0;
unsigned long TaskManager::lastMemCheck = 0;
