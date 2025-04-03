#include "IAYEvent.h"
#include "AYEventRegistry.h"

AYEventRegistry& AYEventRegistry::getInstance()
{
    static AYEventRegistry registry;
    return registry;
}

std::unique_ptr<IAYEvent> AYEventRegistry::create(const std::type_index& typeIdx)
{
    auto it = _creators.find(typeIdx);
    if (it != _creators.end()) {
        return it->second();
    }
    return nullptr;
}

std::unique_ptr<IAYEvent> AYEventRegistry::create(const std::string& typeName)
{
    auto it = _nameToType.find(typeName);
    if (it != _nameToType.end()) {
        return create(it->second);
    }
    return nullptr;
}

bool AYEventRegistry::isRegistered(const std::type_index& typeIdx) const
{
    return _creators.find(typeIdx) != _creators.end();
}
