#pragma once
#include <string>
class AYGameObject;

class IAYComponent 
{
public:
    virtual ~IAYComponent() = default;
    virtual void init() = 0;
    virtual void update(float delta_time) = 0;

    std::string displayName = "BaseComponent"; //用于树状结构展示


    AYGameObject* getOwner() const { return _owner; }
    void setOwner(AYGameObject* owner) { _owner = owner; }
protected:
    AYGameObject* _owner;
};