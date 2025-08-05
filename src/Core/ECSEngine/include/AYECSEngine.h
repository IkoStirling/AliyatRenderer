#pragma once
#include "Mod_ECSEngine.h"
#include <vector>
#include <unordered_map>
#include <memory>
#include <typeindex>
#include <set>
#include <algorithm>
#include <unordered_set>

using EntityID = uint32_t;

// 通用标签组件模板
template<typename T>
struct Tag {};

class AYECSEngine : public Mod_ECSEngine
{
private:
    // 组件存储基础接口
    struct IComponentPool
    {
        virtual ~IComponentPool() = default;
        virtual void remove(EntityID entity) = 0;
    };

    // 特定类型组件的存储
    template<typename T>
    class ComponentPool : public IComponentPool
    {
    public:
        std::unordered_set<EntityID> owningEntities;
        std::unordered_map<EntityID, size_t> entityToIndex;
        std::unordered_map<size_t, EntityID> indexToEntity;
        std::vector<T> components;

        void add(EntityID entity, T&& component);

        void remove(EntityID entity) override;

        T& get(EntityID entity);

        bool has(EntityID entity);
    };

    // 实体管理
    std::unordered_map<std::type_index, std::unique_ptr<IComponentPool>> _componentPools;
    std::set<EntityID> _activeEntities;
    std::set<EntityID> _freeEntities;
    EntityID _nextID = 0;

    template<typename T>
    ComponentPool<T>* getPool();


public:
    AYECSEngine() = default;
    ~AYECSEngine() = default;
    void init() override;
    void shutdown() override;
    void update(float delta_time) override;

    // 创建实体
    EntityID createEntity();

    // 销毁实体
    void destroyEntity(EntityID entity);

    // 添加组件
    template<typename T>
    void addComponent(EntityID entity, T&& component = T{});

    // 移除组件
    template<typename T>
    void removeComponent(EntityID entity);

    // 获取组件
    template<typename T>
    T& getComponent(EntityID entity);


    // 检查是否有组件
    template<typename T>
    bool hasComponent(EntityID entity);


    // 创建组件视图
    template<typename... Components>
    auto getView();

    // 特化空标签的快速创建
    template<typename TagType>
    void addTag(EntityID entity);


private:
    // 视图类定义
    template<typename... Components>
    class View
    {
    public:
        View(AYECSEngine& world, std::vector<EntityID>&& entities)
            : world(world), entities(std::move(entities)) {
        }

        // 迭代器支持
        class Iterator
        {
        public:
            Iterator(AYECSEngine& world, std::vector<EntityID>::iterator it)
                : world(world), it(it) {
            }

            auto operator*() {
                EntityID entity = *it;
                return std::tuple_cat(
                    std::make_tuple(entity), // 添加EntityID到元组首部
                    std::tie(world.getComponent<Components>(entity)...));
            }

            Iterator& operator++() { ++it; return *this; }
            bool operator!=(const Iterator& other) const { return it != other.it; }
            bool operator==(const Iterator& other) const { return it == other.it; }

        private:
            AYECSEngine& world;
            std::vector<EntityID>::iterator it;
        };

        Iterator begin() { return Iterator(world, entities.begin()); }
        Iterator end() { return Iterator(world, entities.end()); }

    private:
        AYECSEngine& world;
        std::vector<EntityID> entities;
    };
};

REGISTER_MODULE_CLASS("ECSEngine", AYECSEngine)

template<typename T>
inline AYECSEngine::ComponentPool<T>* AYECSEngine::getPool()
{
    auto type = std::type_index(typeid(T));
    auto it = _componentPools.find(type); // 改用 find 而不是 at
    return (it != _componentPools.end()) ?
        static_cast<ComponentPool<T>*>(it->second.get()) :
        nullptr;
}

template<typename T>
inline void AYECSEngine::ComponentPool<T>::add(EntityID entity, T&& component)
{
    size_t index = components.size();
    owningEntities.insert(entity);
    entityToIndex[entity] = index;
    indexToEntity[index] = entity;
    components.emplace_back(std::forward<T>(component));
}

template<typename T>
inline void AYECSEngine::ComponentPool<T>::remove(EntityID entity)
{
    if (entityToIndex.count(entity) == 0) return;

    // 用最后一个元素覆盖要删除的元素
    size_t indexToRemove = entityToIndex[entity];
    size_t lastIndex = components.size() - 1;

    if (indexToRemove != lastIndex)
    {
        components[indexToRemove] = std::move(components.back());
        EntityID lastEntity = indexToEntity[lastIndex];
        entityToIndex[lastEntity] = indexToRemove;
        indexToEntity[indexToRemove] = lastEntity;
    }
    
    // 移除最后一个元素
    owningEntities.erase(entity);
    entityToIndex.erase(entity);
    indexToEntity.erase(lastIndex);
    components.pop_back();
}

template<typename T>
inline T& AYECSEngine::ComponentPool<T>::get(EntityID entity)
{
    return components[entityToIndex.at(entity)];
}

template<typename T>
inline bool AYECSEngine::ComponentPool<T>::has(EntityID entity)
{
    return entityToIndex.count(entity) > 0;
}

template<typename T>
inline void AYECSEngine::addComponent(EntityID entity, T&& component)
{
    if (_activeEntities.count(entity) == 0) return;

    auto type = std::type_index(typeid(T));
    if (_componentPools.count(type) == 0)
    {
        _componentPools[type] = std::make_unique<ComponentPool<T>>();
    }

    static_cast<ComponentPool<T>*>(_componentPools[type].get())
        ->add(entity, std::forward<T>(component));  //std::forward只是一个优化，右值移动构造，左值拷贝构造
}

template<typename T>
inline void AYECSEngine::removeComponent(EntityID entity)
{
    if (auto pool = getPool<T>()) pool->remove(entity);
}

template<typename T>
inline T& AYECSEngine::getComponent(EntityID entity)
{
    return getPool<T>()->get(entity); 
}

template<typename T>
inline bool AYECSEngine::hasComponent(EntityID entity)
{
    auto type = std::type_index(typeid(T));
    return _componentPools.count(type) > 0 &&
        _componentPools[type]->has(entity);
}

template<typename ...Components>
inline auto AYECSEngine::getView()
{
    auto getSmallestSet = [this](auto... pools) {
        const std::unordered_set<EntityID>* smallest = nullptr;
        size_t minSize = std::numeric_limits<size_t>::max();
        ((pools && pools->owningEntities.size() < minSize ?
            (minSize = pools->owningEntities.size(), smallest = &pools->owningEntities) : false), ...);
        return smallest;
        };

    auto* smallestPool = getSmallestSet(getPool<Components>()...);
    std::vector<EntityID> result;
    if (!smallestPool) {
        return View<Components...>(*this, std::move(result)); // 返回空 View
    }

    for (EntityID entity : *smallestPool) {
        if ((getPool<Components>()->owningEntities.count(entity) && ...)) {
            result.push_back(entity);
        }
    }
    return View<Components...>(*this, std::move(result));
}

template<typename TagType>
inline void AYECSEngine::addTag(EntityID entity)
{
    addComponent<Tag<TagType>>(entity);
}