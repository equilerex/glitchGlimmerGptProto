#pragma once
#include <Encoder.h>
#include "../core/SettingsManager.h"

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
            Serial.println("Error: Encoder initialization failed!");
            return;
        }
        pinMode(buttonPin, INPUT_PULLUP);
    }

    void update() {
        if (!encoder) return; // Ensure encoder is initialized

        int newEncoderVal = encoder->read() / 4;
        if (newEncoderVal != lastEncoderValue) {
            int delta = newEncoderVal - lastEncoderValue;
            lastEncoderValue = newEncoderVal;
            lastInputTime = millis();
            inSettingsMode = true;
            adjustCurrentSetting(delta);
        }

        if (digitalRead(buttonPin) == LOW) {
            delay(100);  // debounce
            if (digitalRead(buttonPin) == LOW) {
                inSettingsMode = true;
                settings.next(); // Go to next setting
                lastInputTime = millis();
                while (digitalRead(buttonPin) == LOW); // wait for release
            }
        }

        if (inSettingsMode && millis() - lastInputTime > 3000) {
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

            display->showSetting(settingName, settings.get(currentSetting));
        } else if (inSettingsMode && display == nullptr) {
            Serial.println("Warning: DisplayManager is not set!");
        }
    }

private:
    void adjustCurrentSetting(int delta) {
        settings.adjust(delta);
    }
};
