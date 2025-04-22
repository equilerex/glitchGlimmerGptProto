#pragma once
#include <Arduino.h>
#include <driver/i2s.h>
#include <arduinoFFT.h>
#include "../config/Config.h"

struct AudioFeatures {
    float volume;
    float bass, mid, treble;
    float bpm;
    float loudness;
    bool beatDetected;
    const int16_t* waveform;
};

class AudioProcessor {
public:
    AudioProcessor();
    void begin();
    void captureAudio();
    AudioFeatures analyzeAudio();

private:
    double vReal[NUM_SAMPLES];
    double vImag[NUM_SAMPLES];
    int16_t buffer[NUM_SAMPLES];
    ArduinoFFT<double>* FFT;
    float previousVolume;
    float normalizedVolume;
    float currentBPM;
    unsigned long lastBeatTime;
};