#include "AudioProcessor.h"
#include "Config.h"
#include "../core/Debug.h"

AudioProcessor::AudioProcessor()
    : previousVolume(0.0), lastBeatTime(0), currentBPM(0.0),
      normalizedVolume(0.0), rollingMin(1.0), rollingMax(0.0),
      FFT(nullptr), zeroSamplesCount(0), microphoneError(false)
{
    FFT = new ArduinoFFT<double>(vReal, vImag, NUM_SAMPLES, SAMPLE_RATE);
}

AudioProcessor::~AudioProcessor() {
    delete FFT;
}

void AudioProcessor::begin() {
    const i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = 0,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    const i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = -1,
        .data_in_num = I2S_SD
    };

    i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_PORT, &pin_config);
    i2s_start(I2S_PORT);
}

void AudioProcessor::resetI2S() {
    Serial.println("[AudioProcessor] Resetting I2S interface");

    // Uninstall the current driver
    i2s_driver_uninstall(I2S_PORT);

    // Reconfigure I2S
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S, // Updated format
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

    // Reinstall the driver
    esp_err_t result = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    if (result != ESP_OK) {
        Serial.printf("[AudioProcessor] Error reinstalling I2S driver: %d\n", result);
    }

    result = i2s_set_pin(I2S_PORT, &pin_config);
    if (result != ESP_OK) {
        Serial.printf("[AudioProcessor] Error resetting I2S pins: %d\n", result);
    }

    // Discard any initial data
    size_t bytesRead = 0;
    int32_t dummyBuffer[64];
    i2s_read(I2S_PORT, dummyBuffer, sizeof(dummyBuffer), &bytesRead, 0);

    Serial.println("[AudioProcessor] I2S reset complete");
}

void AudioProcessor::captureAudio() {
    Serial.println("[AudioProcessor] captureAudio() called");
    size_t bytesRead = 0;
    static int32_t i2sBuffer[NUM_SAMPLES]; // Static buffer to avoid stack overuse

    // Clear buffer before reading to detect if read operation fails
    memset(i2sBuffer, 0, sizeof(i2sBuffer));

    // Read from I2S with 100ms timeout instead of portMAX_DELAY to prevent hanging
    esp_err_t read_result = i2s_read(I2S_PORT, (void*)i2sBuffer, sizeof(i2sBuffer), &bytesRead, 100 / portTICK_PERIOD_MS);

    if (read_result != ESP_OK) {
        Serial.printf("[AudioProcessor] Error reading from I2S: %d\n", read_result);
    }

    int samplesRead = bytesRead / sizeof(int32_t);

    // Check if we're getting any non-zero data
    bool hasNonZeroData = false;
    for (int i = 0; i < samplesRead; i++) {
        if (i2sBuffer[i] != 0) {
            hasNonZeroData = true;
            break;
        }
    }

    if (!hasNonZeroData) {
        zeroSamplesCount++;
        if (zeroSamplesCount >= MAX_ZERO_SAMPLES_THRESHOLD) {
            microphoneError = true;
            Debug::log(Debug::ERROR, "No audio input detected - check microphone connection");
            resetI2S();  // Try to recover
            zeroSamplesCount = 0;  // Reset counter after recovery attempt
        }
    } else {
        zeroSamplesCount = 0;
        microphoneError = false;
    }

    if (!hasNonZeroData) {
        Serial.println("[AudioProcessor] WARNING: All samples are zero - check microphone connection");
    }

    // Print first few samples for debugging
    for (int i = 0; i < 10 && i < samplesRead; i++) {
        Serial.printf("i2sBuffer[%d]=%ld\n", i, i2sBuffer[i]);
    }

    // Calculate maximum sample value to check signal level
    int32_t maxSample = 0;
    for (int i = 0; i < samplesRead && i < NUM_SAMPLES; i++) {
        int32_t absVal = abs(i2sBuffer[i]);
        if (absVal > maxSample) maxSample = absVal;

        // INMP441 and similar I2S mics produce 24-bit audio in MSB format
        // For proper normalization we need to:
        // 1) Left-align the 24-bit value to get proper sign
        // 2) Then convert to float in -1.0 to 1.0 range
        float normalized = (float)i2sBuffer[i] / 8388608.0f;  // 2^23 = 8388608
        vReal[i] = normalized;
        vImag[i] = 0.0;
        buffer[i] = (int16_t)(normalized * 32767);  // For waveform
    }

    Serial.printf("[AudioProcessor] Max sample value: %ld (%.2f%%)\n",
                 maxSample, (maxSample * 100.0 / 8388608.0));

    // Fallback fill if we couldn't read enough samples
    if (samplesRead < NUM_SAMPLES) {
        Serial.printf("[AudioProcessor] Warning: Only read %d of %d samples\n", samplesRead, NUM_SAMPLES);
        memset(vReal + samplesRead, 0, (NUM_SAMPLES - samplesRead) * sizeof(double));
        memset(vImag + samplesRead, 0, (NUM_SAMPLES - samplesRead) * sizeof(double));
        memset(buffer + samplesRead, 0, (NUM_SAMPLES - samplesRead) * sizeof(int16_t));
    }

    Serial.printf("[AudioProcessor] samplesRead: %d\n", samplesRead);
}

AudioFeatures AudioProcessor::analyzeAudio() {
    if (microphoneError) {
        Debug::log(Debug::ERROR, "Skipping audio analysis due to microphone error");
        AudioFeatures emptyFeatures = {};  // Return zero-initialized features
        return emptyFeatures;
    }

    Serial.println("[AudioProcessor] analyzeAudio() called");
    AudioFeatures features = {};

    // Set the waveform pointer to the buffer
    features.waveform = buffer;
    Serial.printf("[AudioProcessor] Setting waveform pointer: %p\n", (void*)features.waveform);

    // Volume (RMS)
    double sumSquares = 0.0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        vReal[i] = constrain(vReal[i], -1.0f, 1.0f);
        sumSquares += vReal[i] * vReal[i];
    }

    double rawVolume = sqrt(sumSquares / NUM_SAMPLES);
    normalizedVolume = gainSmoothing * normalizedVolume + (1 - gainSmoothing) * rawVolume;
    features.volume = normalizedVolume;

    // Loudness: scale volume to 0–100
    static float smoothedLoudness = 0;
    float rawLoudness = features.volume * 100.0f;
    smoothedLoudness = 0.9f * smoothedLoudness + 0.1f * rawLoudness;
    features.loudness = constrain(smoothedLoudness, 0, 100);

    Serial.printf("[AudioProcessor] After RMS: vol=%.3f, loud=%d\n", features.volume, features.loudness);

    // Beat detection
    double volumeChange = features.volume - previousVolume;
    unsigned long now = millis();
    if (volumeChange > 0.05 && (now - lastBeatTime) > 250) {
        features.beatDetected = true;
        unsigned long beatInterval = now - lastBeatTime;
        if (beatInterval > 250 && beatInterval < 2000) {
            currentBPM = 60000.0 / beatInterval;
        }
        lastBeatTime = now;
    } else {
        features.beatDetected = false;
    }
    features.bpm = currentBPM;

    // FFT
    if (FFT) {
        FFT->windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
        FFT->compute(FFT_FORWARD);
        FFT->complexToMagnitude();
    }

    // Copy FFT spectrum
    memcpy(features.spectrum, vReal, sizeof(double) * (NUM_SAMPLES / 2));

    // Frequency band bin mapping
    int bassLimit = (int)(200.0 * NUM_SAMPLES / SAMPLE_RATE);
    int midLimit  = (int)(2000.0 * NUM_SAMPLES / SAMPLE_RATE);
    int trebleLimit = NUM_SAMPLES / 2;

    double bassSum = 0.0, midSum = 0.0, trebleSum = 0.0;

    for (int i = 0; i < bassLimit; i++) bassSum += vReal[i];
    for (int i = bassLimit; i < midLimit; i++) midSum += vReal[i];
    for (int i = midLimit; i < trebleLimit; i++) trebleSum += vReal[i];

    // Normalize for display (0.0 – 1.0)
    features.bass = constrain((bassSum / bassLimit) / 100.0, 0.0, 1.0);
    features.mid  = constrain((midSum / (midLimit - bassLimit)) / 80.0, 0.0, 1.0);
    features.treble = constrain((trebleSum / (trebleLimit - midLimit)) / 50.0, 0.0, 1.0);

    Serial.printf("[AudioProcessor] After FFT: bass=%.3f, mid=%.3f, treb=%.3f\n", features.bass, features.mid, features.treble);

#if AUDIO_DEBUG
    Serial.printf("Bands | Bass: %.2f | Mid: %.2f | Treble: %.2f\n",
                  features.bass, features.mid, features.treble);
#endif

    previousVolume = features.volume;
    Serial.printf("[AudioProcessor] Returning features: %p\n", (void*)&features);
    return features;
}