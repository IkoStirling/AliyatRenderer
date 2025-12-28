#pragma once
#include "Component/IAYComponent.h"
#include "Component/IAYRenderComponent.h"
#include "AYRendererManager.h"
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <typeindex>
#include "BasePhy/IAYPhysical.h"

namespace ayt::engine::game
{
    using RendererManager = ::ayt::engine::render::RendererManager;

    class GameObject : public ::ayt::engine::physics::IPhysical
    {
    public:
        GameObject(const std::string& name = "GameObject");
        ~GameObject();

        GameObject(GameObject&&) noexcept;
        GameObject& operator=(GameObject&&) noexcept;

        template<typename T, typename... Args>
        T* addComponent(const std::string& name, Args&&... args);

        template<typename T>
        std::vector<T*> getComponents() const;

        template<typename T>
        T* getComponent() const;

        template<typename T>
        bool hasComponent() const;

        template<typename T>
        void removeComponents();

        template<typename T>
        void removeComponent(const std::string& name);

        virtual void beginPlay();
        virtual void update(float delta_time);
        virtual void endPlay();

        void setName(const std::string& name) { _name = name; }
        const std::string& getName() { return _name; }
        void setActive(bool active);
        bool isActive() const { return _active; }


    protected:
        std::string _name;
        std::unordered_multimap<std::type_index, std::unique_ptr<IComponent>> _components;
        bool _active = true;

    private:
        GameObject(const GameObject&) = delete;
        GameObject& operator=(const GameObject&) = delete;

        void _handleRenderComponents(bool shouldRegister);
    };

    template<typename T, typename ...Args>
    inline T* GameObject::addComponent(const std::string& name, Args && ...args)
    {
        static_assert(std::is_base_of_v<IComponent, T>, "T must inherit from IComponent");

        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        component->setOwner(this);
        component->setName(name);
        T* rawPtr = component.get();

        if constexpr (std::is_base_of_v<IRenderComponent, T>)
        {
            GET_CAST_MODULE(RendererManager, "Renderer")->registerRenderable(rawPtr);
        }

        _components.emplace(std::type_index(typeid(T)), std::move(component));

        return rawPtr;
    }

    template<typename T>
    inline std::vector<T*> GameObject::getComponents() const
    {
        static_assert(std::is_base_of_v<IComponent, T>, "T must inherit from IComponent");

        std::vector<T*> result;
        auto range = _components.equal_range(std::type_index(typeid(T)));

        for (auto it = range.first; it != range.second; ++it) {
            result.push_back(static_cast<T*>(it->second.get()));
        }

        return result;
    }

    template<typename T>
    inline T* GameObject::getComponent() const
    {
        auto components = getComponents<T>();
        return components.empty() ? nullptr : components.front();
    }

    template<typename T>
    inline bool GameObject::hasComponent() const
    {
        return _components.count(std::type_index(typeid(T))) > 0;
    }

    template<typename T>
    inline void GameObject::removeComponents()
    {
        if constexpr (std::is_base_of_v<IRenderComponent, T>)
        {
            auto components = getComponents<T>();
            for (auto component : components)
                GET_CAST_MODULE(RendererManager, "Renderer")->removeRenderable(component);
        }

        _components.erase(std::type_index(typeid(T)));
    }

    template<typename T>
    inline void GameObject::removeComponent(const std::string& name)
    {

    }
}