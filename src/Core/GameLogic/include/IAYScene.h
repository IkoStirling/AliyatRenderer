#pragma once
#include "AYEntrant.h"

class IAYScene {
public:
    virtual ~IAYScene() = default;
    virtual void load() = 0;       // 同步加载
    virtual void unload() = 0;
    virtual void beginPlay() = 0;
    virtual void update(float delta_time) = 0;
    virtual void endPlay() = 0;
    virtual void render() = 0;

    // 异步加载接口
    virtual bool isLoaded() const = 0;
    virtual float getLoadProgress() const = 0;

    // 对象管理
    virtual void AddEntrant(std::unique_ptr<AYEntrant> entrant) = 0;
    virtual void RemoveEntrant(AYEntrant* entrant) = 0;

    // 查找功能
    virtual AYEntrant* FindEntrantByName(const std::string& name) = 0;
    template<typename T>
    T* FindEntrantByType();

    // 场景信息
    virtual const std::string& GetName() const = 0;
};