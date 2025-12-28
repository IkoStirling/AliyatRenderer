#include "AYInputBinding.h"
#include "AYInputSystem.h"
namespace ayt::engine::input
{
    InputBinding::~InputBinding()
    {
    }

    void InputBinding::addAction(const std::string& name,
        InputAction::Type type,
        UniversalInput input,
        float long_pressDuration,
        float double_pressDuration)
    {
        _actions.emplace(name, InputAction(type, input, long_pressDuration, double_pressDuration));
    }

    bool InputBinding::isActive(const std::string& name, const InputSystem& input) const
    {
        if (auto it = _actions.find(name); it != _actions.end()) {
            return it->second.evaluate(input);
        }
        return false;
    }

    void InputBinding::rebindAction(const std::string& name, UniversalInput input)
    {
        if (auto it = _actions.find(name); it != _actions.end()) {
            it->second.rebind(input);
        }
    }

    std::vector<std::string> InputBinding::getActionNames() const
    {
        std::vector<std::string> names;
        for (auto& pair : _actions)
        {
            names.push_back(pair.first);
        }
        return names;
    }

    const std::unordered_map<std::string, InputAction>& InputBinding::getActions() const
    {
        return _actions;
    }

    const InputAction& InputBinding::getAction(const std::string& name) const
    {
        if (auto it = _actions.find(name); it != _actions.end())
        {
            return it->second;
        }
        throw std::out_of_range("[InputAction] Action '" + name + "' not found");
    }

    bool InputBinding::hasAction(const std::string& name) const
    {
        return  _actions.find(name) != _actions.end();
    }
}