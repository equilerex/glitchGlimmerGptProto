#pragma once
#include <Arduino.h>
#include "../config/Config.h"

class Debug {
public:
    enum Level {
        ERROR = 0,
        INFO = 1,
        DEBUG = 2
    };

    static void begin(unsigned long baud = DEBUG_BAUDRATE) {
        #if DEBUG_ENABLED
        Serial.begin(baud);
        while(!Serial) delay(10);
        currentLevel = static_cast<Level>(DEBUG_LEVEL);
        #endif
    }

    static void log(Level level, const char* msg) {
        #if DEBUG_ENABLED
        if (level > currentLevel) return;
        unsigned long timestamp = millis();
        Serial.printf("[%lu ms][%s] %s\n", timestamp, getLevelString(level), msg);
        #endif
    }

    static void logf(Level level, const char* format, ...) {
        #if DEBUG_ENABLED
        if (level > currentLevel) return;
        char buf[256];
        va_list args;
        va_start(args, format);
        vsnprintf(buf, sizeof(buf), format, args);
        va_end(args);
        log(level, buf);
        #endif
    }

    static void setLevel(Level level) {
        #if DEBUG_ENABLED
        currentLevel = level;
        #endif
    }

private:
    static Level currentLevel;  // Just declare the static member
    
    static const char* getLevelString(Level level) {
        switch(level) {
            case ERROR: return "ERROR";
            case INFO:  return "INFO ";
            case DEBUG: return "DEBUG";
            default:    return "?????";
        }
    }
};
