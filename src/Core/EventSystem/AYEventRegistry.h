#pragma once
#include <unordered_map>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <functional>

class IAYEvent;

class AYEventRegistry 
{
public:
    static AYEventRegistry& getInstance();

    template<typename T, typename... Args>
    void registerEvent(const std::string& typeName, Args&&... args);

    std::unique_ptr<IAYEvent> create(const std::type_index& typeIdx);

    std::unique_ptr<IAYEvent> create(const std::string& typeName);

    bool isRegistered(const std::type_index& typeIdx) const;

private:
    std::unordered_map<std::type_index, std::function<std::unique_ptr<IAYEvent>()>> _creators;
    std::unordered_map<std::string, std::type_index> _nameToType;
};

template<typename T, typename... Args>
inline void AYEventRegistry::registerEvent(const std::string& typeName, Args&&... args)
{
    static_assert(std::is_base_of_v<IAYEvent, T>, "T must be IAYEvent's children");
    const std::type_info& ti = typeid(T);
    std::type_index typeIdx(ti);
    _creators[typeIdx] = [args...]()mutable { return std::make_unique<T>(args...); };
    _nameToType.try_emplace(typeName,typeIdx);
}


