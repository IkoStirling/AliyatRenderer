#include "IAYEvent.h"
#include "AYEventRegistry.h"
#include "AYEventSystem.h"

AYEventRegistry& AYEventRegistry::getInstance()
{
    static AYEventRegistry registry;
    return registry;
}

IAYEvent* AYEventRegistry::create(const std::type_index& typeIdx)
{
    auto it = _creators.find(typeIdx);
    if (it != _creators.end()) {
        return it->second();
    }
    return nullptr;
}

IAYEvent* AYEventRegistry::create(const std::string& typeName)
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

bool AYEventRegistry::isRegistered(const std::string& typeName) const
{
    return _nameToType.find(typeName) != _nameToType.end();
}
#include <assert.h>
void AYEventRegistry::publish(std::shared_ptr<AYEventSystem> system, const std::string& typeName, std::function<void(IAYEvent*)> wrapped)
{
    assert(getInstance().isRegistered(typeName));

    auto event = getInstance().create(typeName);
    wrapped(event);
    system->publish(std::unique_ptr<IAYEvent>(event));
}
