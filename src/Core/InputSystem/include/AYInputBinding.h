#pragma once
#include "AYInputAction.h"
#include <unordered_map>
#include <string>
#include <vector>

class AYInputBinding 
{
public:
    AYInputBinding() = default;
    ~AYInputBinding();

    void addAction(const std::string& name,
        AYInputAction::Type type,
        UniversalInput input,
        float long_pressDuration = 0.3f,
        float double_pressDuration = 0.3f);

    bool isActive(const std::string& name, const AYInputSystem& input) const;

    void rebindAction(const std::string& name, UniversalInput input);

    std::vector<std::string> getActionNames() const;

    const std::unordered_map<std::string, AYInputAction>& getActions() const;

private:
    std::unordered_map<std::string, AYInputAction> _actions;
};