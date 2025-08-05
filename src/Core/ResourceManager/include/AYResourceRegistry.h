#pragma once
#include "IAYResource.h"
#include <memory>
#include <unordered_map>
#include <functional>
#include <string>
#include <any>

class AYResourceRegistry {
public:
    using ParametricCreatorFunc = std::function<std::shared_ptr<IAYResource>(const std::any&)>;

    static AYResourceRegistry& getInstance() 
    {
        static AYResourceRegistry mInstance;
        return mInstance;
    }

    template<typename T, typename... Args>
    void registerType(const std::string& typeName) 
    {
        _creators[typeName] = [typeName](std::any const& args) -> std::shared_ptr<IAYResource> {
            // 类型安全检查
            try {
                if constexpr (sizeof...(Args) == 0) 
                {
                    return static_pointer_cast<IAYResource>(std::make_shared<T>());
                }
                else
                {
                    const auto& argTuple = std::any_cast<const std::tuple<Args...>&>(args);
                    return std::apply([](auto&&... args) {
                        return std::make_shared<T>(std::forward<decltype(args)>(args)...);
                        }, argTuple); //解包元组并传递到lambda中，返回labmda返回类型
                }
            }
            catch (const std::bad_any_cast& e) 
            {
                throw std::runtime_error("Argument type mismatch for " + typeName);
            }
        };
    }

    template<typename T, typename... Args>
    std::shared_ptr<T> create(const std::string& typeName, Args&&... args) {
        auto it = _creators.find(typeName);
        if (it != _creators.end()) {
            if constexpr (sizeof...(Args) == 0) 
            {
                return std::static_pointer_cast<T>(it->second(std::any()));
            }
            else 
            {
                auto argsTuple = std::make_tuple(std::forward<Args>(args)...);
                return std::static_pointer_cast<T>(it->second(argsTuple));
            }
        }
        return nullptr;
    }

private:
    std::unordered_map<std::string, ParametricCreatorFunc> _creators;
};

