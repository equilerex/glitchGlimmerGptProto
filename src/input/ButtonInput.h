#pragma once
#include <Arduino.h>
#include "../config/Config.h"
#include <Button2.h>
#include "../core/HybridController.h"

class ButtonInput {
private:
    uint8_t buttonPin1;
    uint8_t buttonPin2;
    bool lastState = HIGH;
    unsigned long lastDebounceTime = 0;
    const unsigned long debounceDelay = 50;
    Button2 nextModeBtn;
    Button2 autoModeBtn;
    HybridController& hybridController;

public:
    ButtonInput(HybridController* hybrid, uint8_t pin1, uint8_t pin2) :
                                    hybridController(*hybrid),
                                    buttonPin1(pin1),
                                    buttonPin2(pin2),
                                    nextModeBtn(pin1),
                                    autoModeBtn(pin2) {}

    void begin() {
        nextModeBtn.setPressedHandler([this](Button2 &btn) {
            hybridController.next();
        });

        autoModeBtn.setPressedHandler([this](Button2 &btn) {
            hybridController.toggleAuto();
        });
    }

    bool update() {
        nextModeBtn.loop();
        autoModeBtn.loop();
        return true;
    }
};
