#pragma once

#include "../animations/BaseAnimation.h"
#include <vector>
#include <memory>

class AnimationManager {
private:
    std::vector<std::unique_ptr<BaseAnimation>> animations;
    int currentIndex = 0;

public:
    void initialize();
    void registerAnimation(std::unique_ptr<BaseAnimation> anim);
    void next();
    void previous();

    void update(const AudioFeatures& features);

    const String getCurrentName() const;
    int getCurrentIndex() const;
    int getAnimationCount() const;
};
