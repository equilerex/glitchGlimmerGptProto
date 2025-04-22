#pragma once

#include <Arduino.h>
#include <driver/i2s.h>
#include <arduinoFFT.h>
#include "../config/Config.h"

#define I2S_PORT I2S_NUM_0

struct AudioFeatures {
    double volume = 0.0;
    double bass = 0.0;
    double mid = 0.0;
    double treble = 0.0;
    bool beatDetected = false;
    double bpm = 0.0;
    int loudness = 0;
    double spectrum[NUM_SAMPLES/2] = {0};
    const int16_t* waveform = nullptr;
};

class AudioProcessor {
public:
    AudioProcessor();
    ~AudioProcessor();

    void begin();
    void captureAudio();
    AudioFeatures analyzeAudio();
    void resetI2S();
    bool hasMicrophoneError() const { return microphoneError; }
    void resetMicrophoneError() { 
        microphoneError = false; 
        zeroSamplesCount = 0;
    }

private:
    // Audio buffers and FFT
    double vReal[NUM_SAMPLES];
    double vImag[NUM_SAMPLES];
    int16_t buffer[NUM_SAMPLES];
    ArduinoFFT<double>* FFT;

    // State tracking
    double previousVolume;
    unsigned long lastBeatTime;
    float currentBPM;
    float normalizedVolume;

    // Dynamic gain control
    float rollingMin;
    float rollingMax;
    float gainSmoothing;

    // Counter for tracking consecutive zero readings
    int zeroReadingCount = 0;
    const int MAX_ZERO_READINGS = 5;

    static const int MAX_ZERO_SAMPLES_THRESHOLD = 5;  // Number of consecutive zero readings before error
    int zeroSamplesCount = 0;
    bool microphoneError = false;
};