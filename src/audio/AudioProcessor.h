#pragma once

#include <Arduino.h>
#include <driver/i2s.h>
#include <arduinoFFT.h>
#include "../config/Config.h"
#include "../core/Debug.h"
#include "AudioFeatures.h"

class AudioProcessor {
private:
    double vReal[NUM_SAMPLES];
    double vImag[NUM_SAMPLES];
    int16_t buffer[NUM_SAMPLES];

    ArduinoFFT<double>* FFT;

    float previousVolume = 0.0;
    float normalizedVolume = 0.0;
    float gainSmoothing = 0.85;

    float rollingMin = 1.0;
    float rollingMax = 0.0;

    unsigned long lastBeatTime = 0;
    float currentBPM = 0.0;

    bool microphoneError = false;
    int zeroSamplesCount = 0;

public:
    AudioProcessor() {
        FFT = new ArduinoFFT<double>(vReal, vImag, NUM_SAMPLES, SAMPLE_RATE);
    }

    ~AudioProcessor() {
        delete FFT;
    }

    void begin() {
        i2s_config_t i2s_config = {
            .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
            .sample_rate = SAMPLE_RATE,
            .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
            .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
            .communication_format = I2S_COMM_FORMAT_STAND_I2S,
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
            .dma_buf_count = 8,
            .dma_buf_len = 64,
            .use_apll = false,
            .tx_desc_auto_clear = false,
            .fixed_mclk = 0
        };

        i2s_pin_config_t pin_config = {
            .bck_io_num = I2S_SCK,
            .ws_io_num = I2S_WS,
            .data_out_num = I2S_PIN_NO_CHANGE,
            .data_in_num = I2S_SD
        };

        i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
        i2s_set_pin(I2S_PORT, &pin_config);
        i2s_zero_dma_buffer(I2S_PORT);
    }

    void resetI2S() {
        i2s_driver_uninstall(I2S_PORT);
        begin();
    }

    void captureAudio() {
        static int32_t i2sBuffer[NUM_SAMPLES];
        memset(i2sBuffer, 0, sizeof(i2sBuffer));
        size_t bytesRead = 0;

        esp_err_t result = i2s_read(I2S_PORT, (void*)i2sBuffer, sizeof(i2sBuffer), &bytesRead, 100 / portTICK_PERIOD_MS);
        if (result != ESP_OK) {
            Debug::logf(Debug::ERROR, "[AudioProcessor] I2S read error: %d", result);
            return;
        }

        int samplesRead = bytesRead / sizeof(int32_t);
        bool hasNonZero = false;

        for (int i = 0; i < samplesRead && i < NUM_SAMPLES; i++) {
            int32_t sample = i2sBuffer[i] >> 8;
            if (sample & 0x800000) sample |= ~0xFFFFFF; // Sign extend
            float normalized = sample / 8388608.0f;
            vReal[i] = normalized;
            vImag[i] = 0.0;
            buffer[i] = (int16_t)(normalized * 32767);
            if (sample != 0) hasNonZero = true;
        }

        if (!hasNonZero) {
            zeroSamplesCount++;
            if (zeroSamplesCount >= 10) {
                microphoneError = true;
                Debug::log(Debug::ERROR, "No audio input detected - resetting I2S");
                resetI2S();
                zeroSamplesCount = 0;
            }
        } else {
            zeroSamplesCount = 0;
            microphoneError = false;
        }
    }

    AudioFeatures analyzeAudio() {
        // captureAudio(); todo: maybe this will work?
        AudioFeatures features = {};
        features.waveform = buffer; // buffer is int16_t[NUM_SAMPLES]

        if (microphoneError) return features;

        double sumSquares = 0.0;
        for (int i = 0; i < NUM_SAMPLES; i++) {
            vReal[i] = constrain(vReal[i], -1.0, 1.0);
            sumSquares += vReal[i] * vReal[i];
        }

        double rawVolume = sqrt(sumSquares / NUM_SAMPLES);
        normalizedVolume = gainSmoothing * normalizedVolume + (1.0 - gainSmoothing) * rawVolume;
        features.volume = normalizedVolume;

        static float smoothedLoudness = 0;
        float rawLoudness = features.volume * 100.0f;
        smoothedLoudness = 0.9f * smoothedLoudness + 0.1f * rawLoudness;
        features.loudness = constrain(smoothedLoudness, 0, 100);

        float delta = features.volume - previousVolume;
        unsigned long now = millis();
        if (delta > 0.05 && (now - lastBeatTime) > 250) {
            features.beatDetected = true;
            unsigned long beatInterval = now - lastBeatTime;
            if (beatInterval > 250 && beatInterval < 2000) {
                currentBPM = 60000.0f / beatInterval;
            }
            lastBeatTime = now;
        } else {
            features.beatDetected = false;
        }
        features.bpm = currentBPM;

        if (FFT) {
            FFT->windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
            FFT->compute(FFT_FORWARD);
            FFT->complexToMagnitude();
        }

        memcpy(features.spectrum, vReal, sizeof(double) * (NUM_SAMPLES / 2));

        int bassLimit = 200 * NUM_SAMPLES / SAMPLE_RATE;
        int midLimit = 2000 * NUM_SAMPLES / SAMPLE_RATE;
        int trebLimit = NUM_SAMPLES / 2;

        double bassSum = 0, midSum = 0, trebSum = 0;
        for (int i = 0; i < bassLimit; i++) bassSum += vReal[i];
        for (int i = bassLimit; i < midLimit; i++) midSum += vReal[i];
        for (int i = midLimit; i < trebLimit; i++) trebSum += vReal[i];

        features.bass = constrain((bassSum / bassLimit) / 100.0, 0.0, 1.0);
        features.mid = constrain((midSum / (midLimit - bassLimit)) / 80.0, 0.0, 1.0);
        features.treble = constrain((trebSum / (trebLimit - midLimit)) / 50.0, 0.0, 1.0);

        previousVolume = features.volume;
        return features;
    }
};
