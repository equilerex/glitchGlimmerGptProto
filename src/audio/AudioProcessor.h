#pragma once

#include <Arduino.h>
#include <driver/i2s.h>
#include "arduinoFFT.h"
#include "../config/Config.h"
#include "AudioFeatures.h"

class AudioProcessor {
public:
    AudioProcessor();
    ~AudioProcessor();

    void begin();
    void captureAudio();
    AudioFeatures analyzeAudio();

    // Volume trend analysis
    bool isBuildUp() const;
    bool isDrop() const;

private:
    ArduinoFFT<double>* FFT;

    double vReal[NUM_SAMPLES];
    double vImag[NUM_SAMPLES];
    int16_t buffer[NUM_SAMPLES];

    float previousVolume;
    float normalizedVolume;
    unsigned long lastBeatTime;
    float currentBPM;

    float rollingMin;
    float rollingMax;

    static constexpr int MAX_ZERO_SAMPLES_THRESHOLD = 5;
    int zeroSamplesCount;
    bool microphoneError;

    // Volume history buffer for trend detection
    static constexpr int VOLUME_HISTORY_SIZE = 64;
    float volumeHistory[VOLUME_HISTORY_SIZE];
    int volumeHistoryIndex;
    int volumeHistoryCount;
};
