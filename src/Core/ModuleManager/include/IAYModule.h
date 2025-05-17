#pragma once
#include <memory>

#define REGISTER_MODULE_CLASS(CLASS_NAME, ...) \
namespace { \
struct CLASS_NAME##_Register { \
    CLASS_NAME##_Register() { \
        static bool registered = []() -> bool { \
            try { \
                auto& manager = ::AYModuleManager::getInstance(); \
                if (!manager.hasModule(#CLASS_NAME)) { \
                    return manager.registerModule(#CLASS_NAME, \
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

class IAYModule
{
public:
	virtual void init() = 0;
	virtual void update() = 0;

};