﻿#include "AYECSEngine.h"

void AYECSEngine::init()
{
}

void AYECSEngine::update(float delta_time)
{
}

void AYECSEngine::shutdown()
{
    std::unordered_map<std::type_index, std::unique_ptr<IComponentPool>> cp;
    std::set<EntityID> ae;
    std::set<EntityID> fe;

    cp.swap(_componentPools);
    ae.swap(_activeEntities);
    fe.swap(_freeEntities);
}

EntityID AYECSEngine::createEntity()
{
    EntityID id = _freeEntities.empty() ? _nextID++ : *_freeEntities.begin();
    if (!_freeEntities.empty()) _freeEntities.erase(id);
    _activeEntities.insert(id);
    return id;
}

void AYECSEngine::destroyEntity(EntityID entity)
{
    if (_activeEntities.count(entity) == 0) return;

    for (auto& [type, pool] : _componentPools)
        pool->remove(entity);

    _activeEntities.erase(entity);
    _freeEntities.insert(entity);
}