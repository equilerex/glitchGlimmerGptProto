#include "AudioProcessor.h"
#include "../core/Debug.h"
#include "../config/Config.h"

AudioProcessor::AudioProcessor()
    : FFT(nullptr), previousVolume(0.0), normalizedVolume(0.0),
      currentBPM(0.0), lastBeatTime(0) {
    FFT = new ArduinoFFT<double>(vReal, vImag, NUM_SAMPLES, SAMPLE_RATE);
}

void AudioProcessor::begin() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = 0,
        .dma_buf_count = 4,     // Reduced from 8
        .dma_buf_len = 32,      // Reduced from 64
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = -1,
        .data_in_num = I2S_SD
    };

    i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_PORT, &pin_config);
    i2s_start(I2S_PORT);
}

void AudioProcessor::captureAudio() {
    size_t bytesRead = 0;
    int32_t i2sBuffer[NUM_SAMPLES];
    esp_err_t err = i2s_read(I2S_PORT, (void*)i2sBuffer, sizeof(i2sBuffer), &bytesRead, portMAX_DELAY);
    
    if (err != ESP_OK) {
        Debug::logf(Debug::ERROR, "I2S read failed: %d", err);
        return;
    }

    if (bytesRead == 0) {
        Debug::log(Debug::DEBUG, "No audio data received");
        return;
    }

    int samplesRead = bytesRead / sizeof(int32_t);
    Debug::logf(Debug::DEBUG, "Read %d audio samples", samplesRead);

    for (int i = 0; i < samplesRead && i < NUM_SAMPLES; i++) {
        float normalized = i2sBuffer[i] / 8388608.0f;
        vReal[i] = normalized;
        vImag[i] = 0.0;
        buffer[i] = (int16_t)(normalized * 32767);
    }
    for (int i = samplesRead; i < NUM_SAMPLES; i++) {
        vReal[i] = 0.0;
        vImag[i] = 0.0;
        buffer[i] = 0;
    }
}

AudioFeatures AudioProcessor::analyzeAudio() {
    AudioFeatures features = {};
    
    // Validate input data
    bool hasValidData = false;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        if (vReal[i] != 0.0) {
            hasValidData = true;
            break;
        }
    }

    if (!hasValidData) {
        Debug::log(Debug::DEBUG, "No valid audio data for analysis");
        return features;
    }

    double sumSquares = 0.0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        vReal[i] = constrain(vReal[i], -1.0f, 1.0f);
        sumSquares += vReal[i] * vReal[i];
    }
    double rawVolume = sqrt(sumSquares / NUM_SAMPLES);
    normalizedVolume = 0.9 * normalizedVolume + 0.1 * rawVolume;
    features.volume = normalizedVolume;
    features.loudness = constrain(normalizedVolume * 100.0f, 0.0f, 100.0f);

    double volumeChange = features.volume - previousVolume;
    unsigned long now = millis();
    if (volumeChange > 0.05 && (now - lastBeatTime) > 250) {
        features.beatDetected = true;
        currentBPM = 60000.0 / (now - lastBeatTime);
        lastBeatTime = now;
    } else {
        features.beatDetected = false;
    }
    features.bpm = currentBPM;

    FFT->windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT->compute(FFT_FORWARD);
    FFT->complexToMagnitude();

    int bassLimit = (int)(200.0 * NUM_SAMPLES / SAMPLE_RATE);
    int midLimit = (int)(2000.0 * NUM_SAMPLES / SAMPLE_RATE);

    double bassSum = 0.0, midSum = 0.0, trebleSum = 0.0;
    for (int i = 0; i < bassLimit; i++) bassSum += vReal[i];
    for (int i = bassLimit; i < midLimit; i++) midSum += vReal[i];
    for (int i = midLimit; i < NUM_SAMPLES / 2; i++) trebleSum += vReal[i];

    features.bass = bassSum / bassLimit;
    features.mid = midSum / (midLimit - bassLimit);
    features.treble = trebleSum / (NUM_SAMPLES / 2 - midLimit);
    features.waveform = buffer;

    previousVolume = features.volume;

    if (features.beatDetected) {
        Debug::logf(Debug::DEBUG, "Beat detected! BPM: %.1f", features.bpm);
    }

    return features;
}