#pragma once
#include "IAYResource.h"
#include <memory>
#include <unordered_map>/
#include <functional>
#include <string>

class AYResourceRegistry {
public:
    using CreatorFunc = std::function<std::shared_ptr<IAYResource>()>;

    static AYResourceRegistry& getInstance() {
        static AYResourceRegistry mInstance;
        return mInstance;
    }

    void registerType(const std::string& typeName, CreatorFunc func) {
        _creators[typeName] = func;
    }

    std::shared_ptr<IAYResource> create(const std::string& typeName) const {
        auto it = _creators.find(typeName);
        return (it != _creators.end()) ? it->second() : nullptr;
    }

private:
    std::unordered_map<std::string, CreatorFunc> _creators;
};