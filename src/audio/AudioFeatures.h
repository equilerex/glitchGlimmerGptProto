#pragma once

#include <stdint.h> 
#include "../config/Config.h"

struct AudioFeatures {
    float volume = 0.0f;
    float bass = 0.0f;
    float mid = 0.0f;
    float treble = 0.0f;
    float bpm = 0.0f;
    int loudness = 0;
    bool beatDetected = false;

    const int16_t* waveform = nullptr;
    int waveformSize;  // Add this field
    double spectrum[NUM_SAMPLES / 2] = {0.0};
};
