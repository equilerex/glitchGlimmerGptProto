#pragma once

#include "BaseAnimation.h"

// Include all animations
#include "animations.h"

// Add new animation includes here as needed...

#include <vector>
#include <memory>
#include "../config/Config.h"  // Include for make_unique fallback implementation

inline std::vector<std::unique_ptr<BaseAnimation>> getAllAnimations() {
    std::vector<std::unique_ptr<BaseAnimation>> animations;

    animations.push_back(make_unique<FirestormAnimation>());
    animations.push_back(make_unique<RippleCascadeAnimation>());
    animations.push_back(make_unique<ColorTunnelAnimation>());
    animations.push_back(make_unique<EnergySwirlAnimation>());
    animations.push_back(make_unique<StrobeMatrixAnimation>());
    animations.push_back(make_unique<BassBloomAnimation>());
    animations.push_back(make_unique<ColorDripAnimation>());
    animations.push_back(make_unique<FrequencyRiverAnimation>());
    animations.push_back(make_unique<PartyPulseAnimation>());
    animations.push_back(make_unique<CyberFluxAnimation>());
    animations.push_back(make_unique<BioSignalAnimation>());
    animations.push_back(make_unique<ChaosEngineAnimation>());
    animations.push_back(make_unique<GalacticDriftAnimation>());
    animations.push_back(make_unique<AudioStormAnimation>());
    animations.push_back(make_unique<SpectrumWavesAnimation>());
       // animations.push_back(make_unique<BassPulseAnimation>());


    return animations;
}
