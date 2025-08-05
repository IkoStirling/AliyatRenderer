#pragma once
#include <memory>
#include "AYModuleRegistry.h"

#define DECLARE_MODULE(CLASS_NAME) \
class CLASS_NAME; \
extern void RegisterModule_##CLASS_NAME();

#define REGISTER_MODULE(CLASS_NAME) \
void RegisterModule_##CLASS_NAME() { \
    ::AYModuleManager::getInstance().registerModule(#CLASS_NAME, \
        std::make_shared<CLASS_NAME>()); \
}

//该方式必须要手动引入该头文件才能注册
#define REGISTER_MODULE_CLASS(REGISTER_NAME, CLASS_NAME, ...) \
namespace { \
struct CLASS_NAME##_Register { \
    CLASS_NAME##_Register() { \
        static bool registered = []() -> bool { \
            try { \
                auto& manager = ::AYModuleManager::getInstance(); \
                if (!manager.hasModule(REGISTER_NAME)) { \
                    return manager.registerModule(REGISTER_NAME, \
                        std::make_shared<CLASS_NAME>(##__VA_ARGS__)); \
                } \
                return false; \
            } catch (...) { \
                return false; \
            } \
        }(); \
        (void)registered; \
    } \
}; \
/* 使用函数静态变量避免重复注册 */ \
static int RegisterModule_##CLASS_NAME() { \
    static CLASS_NAME##_Register reg; \
    return 0; \
} \
static int CLASS_NAME##_registered = RegisterModule_##CLASS_NAME(); \
}

/*
    目前使用方式：
        仅为非单例模块提供的单例管理方式
*/
class IAYModule
{
public:
	virtual void init() = 0;
	virtual void update(float delta_time) = 0;
	virtual void shutdown() = 0;

};