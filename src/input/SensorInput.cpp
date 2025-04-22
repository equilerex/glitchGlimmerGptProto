#include "SensorInput.h"

void SensorInput::begin(int pin) {
    sensorPin = pin;
    pinMode(sensorPin, INPUT);
}

void SensorInput::update() {
    int raw = analogRead(sensorPin);
    lastValue = (float)raw / 4095.0f;
}

float SensorInput::getValue() const {
    return lastValue;
}
