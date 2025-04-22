#pragma once

#include "../audio/AudioProcessor.h"
#include "../core/AnimationManager.h"
#include "../utils/Debug.h"

enum class ModeKeepReason {
    NONE,
    NO_BEAT,
    TOO_EARLY,
    UNSTABLE_BEAT,
    BUILDUP,
    RECENT_DROP,
    DISABLED_,
    STABLE_BEAT,
};

class HybridController {
public:
    HybridController(AnimationManager* manager);
    HybridController(AnimationManager& manager);

    void update();                              // No-parameter version for simpler calls
    void update(const AudioFeatures& features);   // Handles switching with audio features
    void render(const AudioFeatures& features);   // Delegates to animation manager

    void next();
    void previous();
    void toggleAuto();
    bool isAutoSwitchEnabled() const;

    const String getCurrentName() const;
    int getCurrentIndex() const;
    int getAnimationCount() const;
    ModeKeepReason getModeKeepReason() const;
    String getModeKeepReasonText() const;

private:
    AnimationManager& animationManager;

    unsigned long lastSwitchTime = 0;
    float volumeHistory[10] = {0};
    int volumePos = 0;
    float smoothedVolume = 0;
    float avgVolume = 0;

    bool autoSwitch = true;
    int debounceCounter = 0;

    bool buildUp = false;
    bool drop = false;

    ModeKeepReason currentReason = ModeKeepReason::NONE;

    bool isBuildUp();
    bool isDrop();
    bool shouldSwitch(const AudioFeatures& features);

    void performSwitch();
};
