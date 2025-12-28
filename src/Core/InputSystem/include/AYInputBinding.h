#pragma once
#include "AYInputAction.h"
#include <unordered_map>
#include <string>
#include <vector>
namespace ayt::engine::input
{
    class InputBinding
    {
    public:
        InputBinding() = default;
        ~InputBinding();

        void addAction(const std::string& name,
            InputAction::Type type,
            UniversalInput input,
            float long_pressDuration = 0.3f,
            float double_pressDuration = 0.3f);

        bool isActive(const std::string& name, const InputSystem& input) const;

        void rebindAction(const std::string& name, UniversalInput input);

        std::vector<std::string> getActionNames() const;

        const std::unordered_map<std::string, InputAction>& getActions() const;

        const InputAction& getAction(const std::string& name) const;

        bool hasAction(const std::string& name) const;

    private:
        std::unordered_map<std::string, InputAction> _actions;
    };
}