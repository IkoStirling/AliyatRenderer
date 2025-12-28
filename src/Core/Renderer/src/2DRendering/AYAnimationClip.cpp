#include "2DRendering/AYAnimationClip.h"
namespace ayt::engine::render
{
    AnimationClip::AnimationClip(const std::string& name, bool isLoop)
        : name(name), isLoop(isLoop) {
    }

    void AnimationClip::addFrame(const AYAnimationFrame& frame) {
        _frames.push_back(frame);
    }

    const AYAnimationFrame& AnimationClip::getFrame(size_t index) const {
        if (index >= _frames.size()) {
            static AYAnimationFrame defaultFrame;
            return defaultFrame;
        }
        return _frames[index];
    }

    size_t AnimationClip::frameCount() const {
        return _frames.size();
    }
}