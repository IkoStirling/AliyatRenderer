#pragma once
#include "AYGameObject.h"
#include <vector>
#include <memory>
#include <string>

namespace ayt::engine::game
{
    AYGameObject::AYGameObject(const std::string& name) :
        _name(name)
    {
    }

    AYGameObject::~AYGameObject()
    {
        _components.clear();
    }

    AYGameObject::AYGameObject(AYGameObject&& other) noexcept :
        _components(std::move(other._components)),
        _active(other._active)
    {
        for (auto& [typeId, component] : _components)
        {
            component->setOwner(this);
        }
    }

    AYGameObject& AYGameObject::operator=(AYGameObject&& other) noexcept
    {
        if (this != &other) {
            _components = std::move(other._components);
            _active = other._active;

            // 更新所有组件的_gameObject指针
            for (auto& [type, component] : _components) {
                component->setOwner(this);
            }
        }
        return *this;
    }

    void AYGameObject::update(float delta_time)
    {
        if (!_active) return;
        for (auto& [type, component] : _components) {
            component->update(delta_time);
        }
    }

    void AYGameObject::beginPlay()
    {
        if (!_active) return;
        for (auto& [type, component] : _components) {
            component->beginPlay();
        }
    }

    void AYGameObject::endPlay()
    {
        if (!_active) return;
        for (auto& [type, component] : _components) {
            component->endPlay();
        }
    }

    void AYGameObject::setActive(bool active)
    {
        if (_active == active) return;

        _active = active;
        _handleRenderComponents(active);

        //此处逻辑可能有误
        if (active)
            beginPlay();
        else
            endPlay();
    }

    void AYGameObject::_handleRenderComponents(bool shouldRegister)
    {
        auto renderer = GET_CAST_MODULE(AYRendererManager, "Renderer");
        if (!renderer) return;

        for (auto& comp : _components) {
            if (auto* renderComp = dynamic_cast<IAYRenderComponent*>(comp.second.get())) {
                if (shouldRegister) {
                    renderer->registerRenderable(renderComp);
                }
                else {
                    renderer->removeRenderable(renderComp);
                }
            }
        }
    }
}