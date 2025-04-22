#include "AudioProcessor.h"
#include "../core/Debug.h"
#include "../config/Config.h"

AudioProcessor::AudioProcessor() {
    // Initialize volume history tracking
    volumeHistoryIndex = 0;
    volumeHistoryCount = 0;
    for (int i = 0; i < VOLUME_HISTORY_SIZE; ++i) {
        volumeHistory[i] = 0.0f;
    }
}

AudioProcessor::~AudioProcessor() {}

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

    // Reset volume history at start
    volumeHistoryIndex = 0;
    volumeHistoryCount = 0;
    for (int i = 0; i < VOLUME_HISTORY_SIZE; ++i) {
        volumeHistory[i] = 0.0f;
    } 
}

void AudioProcessor::captureAudio() {
    size_t bytesRead = 0;
    static int32_t i2sBuffer[NUM_SAMPLES]; // Static buffer to avoid stack overuse
    i2s_read(I2S_PORT, (void*)i2sBuffer, sizeof(i2sBuffer), &bytesRead, portMAX_DELAY);

    int samplesRead = bytesRead / sizeof(int32_t);

    /*for (int i = 0; i < 10 && i < samplesRead; i++) {
        // prove that data is being read
        Serial.printf("i2sBuffer[%d]=%ld\n", i, i2sBuffer[i]);
    }*/

    for (int i = 0; i < samplesRead && i < NUM_SAMPLES; i++) {
        float normalized = i2sBuffer[i] / 8388608.0f;  // Normalize 24-bit signed PCM
        vReal[i] = normalized;
        vImag[i] = 0.0;
        buffer[i] = (int16_t)(normalized * 32767);  // For waveform
    }

    // Fallback fill
    if (samplesRead < NUM_SAMPLES) {
        memset(vReal + samplesRead, 0, (NUM_SAMPLES - samplesRead) * sizeof(double));
        memset(vImag + samplesRead, 0, (NUM_SAMPLES - samplesRead) * sizeof(double));
        memset(buffer + samplesRead, 0, (NUM_SAMPLES - samplesRead) * sizeof(int16_t));
    }
    // prove that data is being read
    // Serial.printf("[AudioProcessor] samplesRead: %d\n", samplesRead);
}

AudioFeatures AudioProcessor::analyzeAudio() {
    AudioFeatures features = {};
    // Ensure we have valid data in vReal (check if any sample is non-zero)
    bool hasValidData = false;
    for (int i = 0; i < NUM_SAMPLES; ++i) {
        if (vReal[i] != 0.0f) { hasValidData = true; break; }
    }
    if (!hasValidData) {
        Debug::log(Debug::DEBUG, "No valid audio data for analysis");
        return features;
    }

    // Compute loudness (volume) via RMS
    double sumSquares = 0.0;
    for (int i = 0; i < NUM_SAMPLES; ++i) {
        // vReal[i] is already constrained to [-1,1]
        sumSquares += vReal[i] * vReal[i];
    }
    double meanSquare = sumSquares / NUM_SAMPLES;
    features.volume = sqrt(meanSquare);  // RMS volume in 0.0 to 1.0 range


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


    // **Update volume history for build-up/drop detection**
    // Add the latest volume to the circular history buffer
    volumeHistory[volumeHistoryIndex] = features.volume;
    volumeHistoryIndex = (volumeHistoryIndex + 1) % VOLUME_HISTORY_SIZE;
    if (volumeHistoryCount < VOLUME_HISTORY_SIZE) {
        volumeHistoryCount++;
    }

    // (Optional: smooth the volume if needed by averaging with previous values – not applied here for responsiveness)

    // Debug log beat detection if any (ensure beat detection logic is preserved)
    if (features.beatDetected) {
        Debug::logf(Debug::DEBUG, "Beat detected! BPM: %.1f", features.bpm);
    }

    return features;
}

bool AudioProcessor::isBuildUp() const {
    // Require sufficient history to judge a trend
    if (volumeHistoryCount < VOLUME_HISTORY_SIZE / 4) {
        // Not enough data (e.g., fewer than 16 samples in a 64-sample buffer)
        return false;
    }
    int halfLen = volumeHistoryCount / 2;
    double olderSum = 0.0, newerSum = 0.0;
    if (volumeHistoryCount == VOLUME_HISTORY_SIZE) {
        // Buffer is full, split into two halves in circular order
        int oldestIndex = volumeHistoryIndex;  // index of the oldest element
        // Sum older half
        for (int i = 0; i < halfLen; ++i) {
            int idx = (oldestIndex + i) % VOLUME_HISTORY_SIZE;
            olderSum += volumeHistory[idx];
        }
        // Sum newer half
        int midIndex = (oldestIndex + halfLen) % VOLUME_HISTORY_SIZE;
        for (int i = 0; i < halfLen; ++i) {
            int idx = (midIndex + i) % VOLUME_HISTORY_SIZE;
            newerSum += volumeHistory[idx];
        }
    } else {
        // Buffer not full yet, use first half vs second half of existing data
        for (int i = 0; i < halfLen; ++i) {
            olderSum += volumeHistory[i];
        }
        for (int i = halfLen; i < volumeHistoryCount; ++i) {
            newerSum += volumeHistory[i];
        }
    }
    double olderAvg = olderSum / halfLen;
    double newerAvg = newerSum / (volumeHistoryCount - halfLen);
    // Avoid division by zero in ratio calculation
    if (olderAvg < 1e-3) {
        olderAvg = 1e-3;
    }

    // Check if volume is significantly higher in newer segment (rising trend)
    const float BUILDUP_RATIO = 1.3f;   // threshold for how much louder new vs old
    const float MIN_VOLUME_DIFF = 0.1f; // minimum absolute difference to filter noise
    bool risingFast = (newerAvg > olderAvg * BUILDUP_RATIO) && ((newerAvg - olderAvg) > MIN_VOLUME_DIFF);
    return risingFast;
}

bool AudioProcessor::isDrop() const {
    // Require sufficient history (same criterion as build-up)
    if (volumeHistoryCount < VOLUME_HISTORY_SIZE / 4) {
        return false;
    }
    int halfLen = volumeHistoryCount / 2;
    double olderSum = 0.0, newerSum = 0.0;
    if (volumeHistoryCount == VOLUME_HISTORY_SIZE) {
        int oldestIndex = volumeHistoryIndex;
        for (int i = 0; i < halfLen; ++i) {
            int idx = (oldestIndex + i) % VOLUME_HISTORY_SIZE;
            olderSum += volumeHistory[idx];
        }
        int midIndex = (oldestIndex + halfLen) % VOLUME_HISTORY_SIZE;
        for (int i = 0; i < halfLen; ++i) {
            int idx = (midIndex + i) % VOLUME_HISTORY_SIZE;
            newerSum += volumeHistory[idx];
        }
    } else {
        for (int i = 0; i < halfLen; ++i) {
            olderSum += volumeHistory[i];
        }
        for (int i = halfLen; i < volumeHistoryCount; ++i) {
            newerSum += volumeHistory[i];
        }
    }
    double olderAvg = olderSum / halfLen;
    double newerAvg = newerSum / (volumeHistoryCount - halfLen);
    if (olderAvg < 1e-3) {
        olderAvg = 1e-3;
    }

    // Check if volume is significantly lower in newer segment (falling trend)
    const float DROP_RATIO = 0.7f;      // threshold for how much quieter new vs old
    const float MIN_VOLUME_DIFF = 0.1f; // minimum absolute difference to filter noise
    bool droppingFast = (newerAvg < olderAvg * DROP_RATIO) && ((olderAvg - newerAvg) > MIN_VOLUME_DIFF);
    return droppingFast;
}
