#pragma once
#include <unordered_map>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <functional>

class IAYEvent;
class AYEventSystem;

/*
    该类作用：
        注册所有事件类型
        通过名称发布事件
*/
class AYEventRegistry 
{
public:
    static AYEventRegistry& getInstance();

    template<typename T, typename... Args>
    void registerEvent(const std::string& typeName, Args&&... args);

    IAYEvent* create(const std::type_index& typeIdx);

    IAYEvent* create(const std::string& typeName);

    bool isRegistered(const std::type_index& typeIdx) const;

    bool isRegistered(const std::string& typeName) const;

    static void publish(const std::string& typeName, std::function<void(IAYEvent*)> wrapped);

private:
    std::unordered_map<std::type_index, std::function<IAYEvent*()>> _creators;
    std::unordered_map<std::string, std::type_index> _nameToType;
};

template<typename T, typename... Args>
inline void AYEventRegistry::registerEvent(const std::string& typeName, Args&&... args)
{
    static_assert(std::is_base_of_v<IAYEvent, T>, "T must be IAYEvent's children.");

    const std::type_info& ti = typeid(T);
    std::type_index typeIdx(ti);
    _creators[typeIdx] = [args...]()mutable { return new T(args...); };
    _nameToType.try_emplace(typeName,typeIdx);
}




