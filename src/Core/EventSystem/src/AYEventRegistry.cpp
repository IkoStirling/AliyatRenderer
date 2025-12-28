#include "IAYEvent.h"
#include "AYEventRegistry.h"
#include "AYEventSystem.h"

namespace ayt::engine::event
{
    EventRegistry& EventRegistry::getInstance()
    {
        static EventRegistry registry;
        return registry;
    }

    std::unique_ptr<IEvent, PoolDeleter> EventRegistry::create(const std::type_index& typeIdx)
    {
        auto it = _creators.find(typeIdx);
        if (it != _creators.end()) {
            return it->second();
        }
        return nullptr;
    }

    std::unique_ptr<IEvent, PoolDeleter> EventRegistry::create(const std::string& typeName)
    {
        auto it = _nameToType.find(typeName);
        if (it != _nameToType.end()) {
            return create(it->second);
        }
        return nullptr;
    }

    bool EventRegistry::isRegistered(const std::type_index& typeIdx) const
    {
        return _creators.find(typeIdx) != _creators.end();
    }

    bool EventRegistry::isRegistered(const std::string& typeName) const
    {
        return _nameToType.find(typeName) != _nameToType.end();
    }

#include <assert.h>
    void EventRegistry::publish(const std::string& typeName, std::function<void(IEvent*)> wrapped)
    {
        assert(getInstance().isRegistered(typeName));

        auto event = getInstance().create(typeName);
        wrapped(event.get());
        auto system = GET_CAST_MODULE(EventSystem, "EventSystem");
        if (system)
            system->publish(std::move(event));
    }
}