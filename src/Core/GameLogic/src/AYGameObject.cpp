#pragma once
#include "AYGameObject.h"
#include <vector>
#include <memory>
#include <string>

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


