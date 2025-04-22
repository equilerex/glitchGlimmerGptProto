#include <FastLED.h>
#pragma once


// ==== AUDIO ====
#define SAMPLE_RATE        44100       // Sampling rate in Hz
#define NUM_SAMPLES        256         // FFT input sample count (power of 2)
#define I2S_PORT           I2S_NUM_0   // Which I2S peripheral to use
#define I2S_SCK            27          // Bit clock pin
#define I2S_WS             26          // Word select (LR clock)
#define I2S_SD             32          // Serial data input (from mic)
#define LED_PIN            25
// ==== LED ====

//cant put this in the array, needs to be defined on compile
#define LED_STRIP_CONFIGS_1_PIN            25
// #define LED_STRIP_CONFIGS_1_PIN          39
// #define LED_STRIP_CONFIGS_1_PIN          38
// #define LED_STRIP_CONFIGS_1_PIN           37

#define DEFAULT_BRIGHTNESS 128         // Default brightness

#define LED_STRIP_PIN_1  25
#define LED_STRIP_COUNT_1  60

// ==== MEMORY MANAGEMENT ====
#define ENABLE_HEAP_MONITORING true
#define MIN_FREE_HEAP         32768    // 32KB minimum free heap

// ==== ENCODER ====
#define ENCODER_PIN_A      22
#define ENCODER_PIN_B      21
#define ENCODER_BTN_PIN    17

// ==== BUTTON ====
#define BUTTON_PIN_1         0
#define BUTTON_PIN_2         35

// ==== OTHER ====
#define ENABLE_WEB_UI      false        // Enable/disable WebUI
#define DEBUG_ENABLED      true        // Toggle debug logging

// ==== DISPLAY ====
#define DISPLAY_WIDTH      240
#define DISPLAY_HEIGHT     135
#define DISPLAY_PIN         4

// ==== VISUALIZATION ====
#define FFT_MAX_SCALE      50.0        // Scale factor for normalizing FFT bars
#define BAR_HEIGHT_MAX     30          // Max height for bass/mid/treble bars

// ==== DEBUG CONFIG ====
#define DEBUG_ENABLED      true        // Master debug switch
#define DEBUG_LEVEL       2           // 0=ERROR, 1=INFO, 2=DEBUG
#define DEBUG_BAUDRATE    115200      // Debug serial baudrate

// Fallback for std::make_unique if not available
#if __cplusplus < 201402L
#include <memory>
template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
#endif
