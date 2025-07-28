#pragma once
#include <unordered_map>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <functional>
#include <memory>

class IAYEvent;
class AYEventSystem;
struct PoolDeleter;
/*
    该类作用：
        注册所有事件类型
        通过名称发布事件

    注意：
        请以此类为事件系统入口！！！
*/
class AYEventRegistry 
{
public:
    static AYEventRegistry& getInstance();

    template<typename T, typename... Args>
    void registerEvent(const std::string& typeName, Args&&... args);

    std::unique_ptr<IAYEvent, PoolDeleter> create(const std::type_index& typeIdx);

    std::unique_ptr<IAYEvent, PoolDeleter> create(const std::string& typeName);

    bool isRegistered(const std::type_index& typeIdx) const;

    bool isRegistered(const std::string& typeName) const;

    // 允许在发布时对事件进行预处理，比如设置值，验证，日志记录等
    static void publish(const std::string& typeName, std::function<void(IAYEvent*)> wrapped);

private:
    std::unordered_map<std::type_index, std::function<std::unique_ptr<IAYEvent, PoolDeleter>()>> _creators; //无参工厂map
    std::unordered_map<std::string, std::type_index> _nameToType;
};

template<typename T, typename... Args>
inline void AYEventRegistry::registerEvent(const std::string& typeName, Args&&... args)
{
    static_assert(std::is_base_of_v<IAYEvent, T>, "T must be IAYEvent's children.");

    const std::type_info& ti = typeid(T);
    std::type_index typeIdx(ti);
    _creators[typeIdx] = [args...]()mutable { return MakeMUnique<T>(args...); };     //内部捕获了参数，因此可以附带构造
    _nameToType.try_emplace(typeName,typeIdx);
}




