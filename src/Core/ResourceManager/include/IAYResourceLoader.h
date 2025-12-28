#pragma once
#include "ResourceManager/IAYResource.h"
#include <memory>
#include <string>
namespace ayt::engine::resource
{
    class IResourceLoader {
    public:
        virtual ~IResourceLoader() = default;
        virtual std::shared_ptr<IResource> load(const std::string& path) = 0;
    };
}