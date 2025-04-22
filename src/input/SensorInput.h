#pragma once
#include <Arduino.h>

class SensorInput {
public:
    void begin(int pin);
    void update();
    float getValue() const;

private:
    int sensorPin = -1;
    float lastValue = 0;
};
