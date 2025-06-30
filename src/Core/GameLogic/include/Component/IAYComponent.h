#pragma once
#include <string>
class AYGameObject;
class IAYComponent 
{
public:
    virtual ~IAYComponent() = default;
    virtual void beginPlay() = 0;
    virtual void update(float delta_time) = 0;
    virtual void endPlay() = 0;

    AYGameObject* getOwner() const { return _owner; }
    void setOwner(AYGameObject* owner) { _owner = owner; }
    void setName(const std::string& name) { _name = name; }
    const std::string& getName() { return _name; }
protected:
    AYGameObject* _owner;
    std::string _name;
};