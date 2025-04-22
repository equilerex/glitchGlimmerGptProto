#pragma once

#include <Arduino.h>
#include "../control/HybridController.h"

class ButtonInput {
public:
    ButtonInput(HybridController* controller, uint8_t pin1, uint8_t pin2)
        : hybrid(controller), button1Pin(pin1), button2Pin(pin2) {}

    void begin() {
        pinMode(button1Pin, INPUT_PULLUP);
        pinMode(button2Pin, INPUT_PULLUP);
    }

    void update() {
        bool b1 = digitalRead(button1Pin) == LOW;
        bool b2 = digitalRead(button2Pin) == LOW;

        unsigned long now = millis();

        if (b1 && now - lastPress1 > debounceDelay) {
            hybrid->previous();
            lastPress1 = now;
        }

        if (b2 && now - lastPress2 > debounceDelay) {
            hybrid->next();
            lastPress2 = now;
        }
    }

private:
    HybridController* hybrid;
    uint8_t button1Pin;
    uint8_t button2Pin;
    unsigned long lastPress1 = 0;
    unsigned long lastPress2 = 0;
    static constexpr unsigned long debounceDelay = 250;
};