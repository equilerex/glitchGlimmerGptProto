#pragma once

#include <Arduino.h>

// Global toggle
#define DEBUG_ENABLED true

// === Debug macros ===
#if DEBUG_ENABLED
  #define DEBUG_PRINT(x)        Serial.print(x)
  #define DEBUG_PRINTLN(x)      Serial.println(x)
  #define DEBUG_PRINTF(...)     Serial.printf(__VA_ARGS__)
  #define DEBUG_BEGIN(baud)     Serial.begin(baud)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(...)
  #define DEBUG_BEGIN(baud)
#endif

// === Optional verbose macro for pinpoint logs ===
#if DEBUG_ENABLED
  #define DEBUG_LOG(label, val) do { Serial.print(label); Serial.print(": "); Serial.println(val); } while(0)
#else
  #define DEBUG_LOG(label, val)
#endif
