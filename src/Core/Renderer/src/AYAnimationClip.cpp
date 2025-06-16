#include "AYAnimationClip.h"

AYAnimationClip::AYAnimationClip(const std::string& name, bool isLoop)
    : name(name), isLoop(isLoop) {
}

void AYAnimationClip::addFrame(const AYAnimationFrame& frame) {
    _frames.push_back(frame);
}

const AYAnimationFrame& AYAnimationClip::getFrame(size_t index) const {
    if (index >= _frames.size()) {
        static AYAnimationFrame defaultFrame;
        return defaultFrame;
    }
    return _frames[index];
}

size_t AYAnimationClip::frameCount() const {
    return _frames.size();
}