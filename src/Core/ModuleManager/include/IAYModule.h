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

//�÷�ʽ����Ҫ�ֶ������ͷ�ļ�����ע��
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
/* ʹ�ú�����̬���������ظ�ע�� */ \
static int RegisterModule_##CLASS_NAME() { \
    static CLASS_NAME##_Register reg; \
    return 0; \
} \
static int CLASS_NAME##_registered = RegisterModule_##CLASS_NAME(); \
}

/*
    Ŀǰʹ�÷�ʽ��
        ��Ϊ�ǵ���ģ���ṩ�ĵ�������ʽ
*/
class IAYModule
{
public:
	virtual void init() = 0;
	virtual void update() = 0;

};