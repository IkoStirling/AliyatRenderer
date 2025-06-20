#pragma once
#include "Component/IAYComponent.h"
#include "Component/IAYRenderComponent.h"
#include "AYRendererManager.h"
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <typeindex>

class AYGameObject 
{
public:
    AYGameObject() = default;
    ~AYGameObject();

    AYGameObject(const AYGameObject&) = delete;
    AYGameObject& operator=(const AYGameObject&) = delete;

    AYGameObject(AYGameObject&&) noexcept;
    AYGameObject& operator=(AYGameObject&&) noexcept;

    template<typename T, typename... Args>
    T* addComponent(Args&&... args);

    template<typename T>
    std::vector<T*> getComponents() const;

    template<typename T>
    T* getComponent() const;

    template<typename T>
    bool hasComponent() const;

    /*
        组件一旦绑定不建议移除，尽管这里依旧会提供接口
    */
    template<typename T>
    void removeComponents();

    virtual void update(float delta_time);

    void setActive(bool active) { _active = active; }
    bool isActive() const { return _active; }

private:
    std::unordered_multimap<std::type_index, std::unique_ptr<IAYComponent>> _components;
    bool _active = true;
};

template<typename T, typename ...Args>
inline T* AYGameObject::addComponent(Args && ...args)
{
    static_assert(std::is_base_of_v<IAYComponent, T>, "T must inherit from IAYComponent");
    
    auto component = std::make_unique<T>(std::forward<Args>(args)...);
    component->setOwner(this);
    T* rawPtr = component.get();
    
    if constexpr (std::is_base_of_v<IAYRenderComponent, T>)
    {
        GET_CAST_MODULE(AYRendererManager, "Renderer")->registerRenderable(rawPtr);
    }

    _components.emplace(std::type_index(typeid(T)), std::move(component));
    rawPtr->init();
    
    return rawPtr;
}

template<typename T>
inline std::vector<T*> AYGameObject::getComponents() const
{
    static_assert(std::is_base_of_v<IAYComponent, T>, "T must inherit from IAYComponent");

    std::vector<T*> result;
    auto range = _components.equal_range(std::type_index(typeid(T)));

    for (auto it = range.first; it != range.second; ++it) {
        result.push_back(static_cast<T*>(it->second.get()));
    }

    return result;
}

template<typename T>
inline T* AYGameObject::getComponent() const
{
    auto components = getComponents<T>();
    return components.empty() ? nullptr : components.front();
}

template<typename T>
inline bool AYGameObject::hasComponent() const
{
    return _components.count(std::type_index(typeid(T))) > 0;
}

template<typename T>
inline void AYGameObject::removeComponents()
{
    if constexpr (std::is_base_of_v<IAYRenderComponent, T>)
    {
        auto components = getComponents<T>();
        for(auto component: components)
            GET_CAST_MODULE(AYRendererManager, "Renderer")->removeRenderable(component);
    }
        
    _components.erase(std::type_index(typeid(T)));
}

