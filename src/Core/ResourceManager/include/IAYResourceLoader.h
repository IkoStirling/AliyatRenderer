#pragma once
#include "ResourceManager/IAYResource.h"
#include <memory>
#include <string>

class IAYResourceLoader {
public:
    virtual ~IAYResourceLoader() = default;
    virtual std::shared_ptr<IAYResource> load(const std::string& path) = 0;
};