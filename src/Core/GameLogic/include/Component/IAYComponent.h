#pragma once
#include <string>

namespace ayt::engine::game
{
    class GameObject;
    class IComponent
    {
    public:
        virtual ~IComponent() = default;
        virtual void beginPlay() = 0;
        virtual void update(float delta_time) = 0;
        virtual void endPlay() = 0;

        GameObject* getOwner() const { return _owner; }
        void setOwner(GameObject* owner) { _owner = owner; }
        void setName(const std::string& name) { _name = name; }
        const std::string& getName() { return _name; }
    protected:
        GameObject* _owner;
        std::string _name;
    };
}