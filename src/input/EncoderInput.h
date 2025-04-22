#pragma once
#include <Encoder.h>
#include "../core/SettingsManager.h"
#include "../core/Debug.h"

// Forward declaration to avoid circular dependency
class DisplayManager;

class EncoderInput {
private:
    Encoder* encoder = nullptr;
    uint8_t pinA;
    uint8_t pinB;
    uint8_t buttonPin;
    DisplayManager* display;
    SettingsManager& settings;

    unsigned long lastInputTime = 0;
    bool inSettingsMode = false;
    int lastEncoderValue = 0;

public:
    EncoderInput(uint8_t pinA, uint8_t pinB, uint8_t btnPin, SettingsManager& settings)
        : pinA(pinA), pinB(pinB), buttonPin(btnPin), display(nullptr), settings(settings) {}

    void setDisplay(DisplayManager* displayMgr) {
        display = displayMgr;
    }

    void begin() {
        encoder = new Encoder(pinA, pinB);
        if (!encoder) {
            Debug::log(Debug::ERROR, "Encoder initialization failed!");
            return;
        }
        pinMode(buttonPin, INPUT_PULLUP);
        Debug::logf(Debug::INFO, "EncoderInput initialized: pins A=%d, B=%d, button=%d",
                    pinA, pinB, buttonPin);
    }

    void update() {
        if (!encoder) return; // Ensure encoder is initialized

        int newEncoderVal = encoder->read() / 4;
        if (newEncoderVal != lastEncoderValue) {
            int delta = newEncoderVal - lastEncoderValue;
            lastEncoderValue = newEncoderVal;
            lastInputTime = millis();
            inSettingsMode = true;

            Debug::logf(Debug::DEBUG, "Encoder rotated: delta=%d, newValue=%d",
                        delta, newEncoderVal);
            adjustCurrentSetting(delta);
        }

        if (digitalRead(buttonPin) == LOW) {
            Debug::log(Debug::DEBUG, "Button press detected, debouncing..."); 
                // todo: debounce
            if (digitalRead(buttonPin) == LOW) {
                inSettingsMode = true;
                Debug::log(Debug::INFO, "Button press confirmed, switching to next setting");
                settings.next(); // Go to next setting
                lastInputTime = millis();
                while (digitalRead(buttonPin) == LOW); // wait for release
                Debug::log(Debug::DEBUG, "Button released");
            }
        }

        if (inSettingsMode && millis() - lastInputTime > 3000) {
            Debug::log(Debug::DEBUG, "Settings mode timeout, returning to normal mode");
            inSettingsMode = false;
        }

        if (inSettingsMode && display != nullptr) {
            Setting currentSetting = settings.getCurrentSetting();
            String settingName;

            // Convert enum to string
            switch(currentSetting) {
                case Setting::BRIGHTNESS: settingName = "Brightness"; break;
                case Setting::SPEED: settingName = "Speed"; break;
                case Setting::HUE: settingName = "Hue"; break;
                case Setting::SATURATION: settingName = "Saturation"; break;
                default: settingName = "Unknown"; break;
            }

            Debug::logf(Debug::DEBUG, "Showing setting: %s = %d",
                       settingName.c_str(), settings.get(currentSetting));
            display->showSetting(settingName, settings.get(currentSetting));
        } else if (inSettingsMode && display == nullptr) {
            Debug::log(Debug::ERROR, "DisplayManager is not set! Cannot show settings.");
        }
    }

private:
    void adjustCurrentSetting(int delta) {
        settings.adjust(delta);
    }
};
