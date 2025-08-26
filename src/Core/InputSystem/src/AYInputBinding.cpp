#include "AYInputBinding.h"
#include "AYInputSystem.h"

AYInputBinding::~AYInputBinding()
{
}

void AYInputBinding::addAction(const std::string& name,
    AYInputAction::Type type,
    UniversalInput input,
    float long_pressDuration,
    float double_pressDuration) 
{
    _actions.emplace(name, AYInputAction(type, input, long_pressDuration, double_pressDuration));
}

bool AYInputBinding::isActive(const std::string& name, const AYInputSystem& input) const 
{
    if (auto it = _actions.find(name); it != _actions.end()) {
        return it->second.evaluate(input);
    }
    return false;
}

void AYInputBinding::rebindAction(const std::string& name, UniversalInput input)
{
    if (auto it = _actions.find(name); it != _actions.end()) {
        it->second.rebind(input);
    }
}

std::vector<std::string> AYInputBinding::getActionNames() const 
{
    std::vector<std::string> names;
    for (auto& pair : _actions)
    {
        names.push_back(pair.first);
    }
    return names;
}

const std::unordered_map<std::string, AYInputAction>& AYInputBinding::getActions() const
{ 
    return _actions;
}

const AYInputAction& AYInputBinding::getAction(const std::string& name) const
{
    if (auto it = _actions.find(name); it != _actions.end())
    {
        return it->second;
    }
    throw std::out_of_range("[InputAction] Action '" + name + "' not found");
}

bool AYInputBinding::hasAction(const std::string& name) const
{
    return  _actions.find(name) != _actions.end();
}