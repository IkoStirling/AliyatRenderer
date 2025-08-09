#pragma once

#include "ECAVIDependence.h"

class IAYAudioSource
{
public:
    virtual const std::vector<uint8_t>& getPCMData() const = 0;
    virtual bool isStreaming() const = 0;
    virtual int getSampleRate() const = 0;
    virtual int getChannels() const = 0;
    virtual float getDuration() const = 0;
    virtual ALenum getFormat() const = 0;
};