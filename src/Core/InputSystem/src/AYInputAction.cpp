#include "AYInputAction.h"
#include "AYInputSystem.h"

AYInputAction::AYInputAction(Type type, UniversalInput input, float long_pressDuration, float double_pressDuration)
    : _type(type), _input(input), _durationThreshold(long_pressDuration), _doublePressThreshold(double_pressDuration)
{
}

bool AYInputAction::evaluate(const AYInputSystem& input) const 
{
    return std::visit([&](auto&& arg) -> bool {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, KeyboardInput> ||
            std::is_same_v<T, MouseButtonInput> ||
            std::is_same_v<T, GamepadButtonInput>) 
        {
            const InputState* state = input.findInputState(_input);
            if (!state) return false;

            bool pressed = state->current;
            float currentTime = (float)glfwGetTime();

            switch (_type)
            {
            case Type::Press:
                return pressed && !state->previous;

            case Type::Release:
                return !pressed && state->previous;

            case Type::Hold:
                return pressed;

            case Type::DoublePress:
                return state->pressCount >= 2 &&
                    (currentTime - state->lastPressTime) < _doublePressThreshold;

            case Type::LongPress:
                return pressed && state->duration >= _durationThreshold;

            default:
                return false;
            }
        }
        else if constexpr (std::is_same_v<T, MouseAxisInput> ||
            std::is_same_v<T, GamepadAxisInput>) 
        {
            float value = input.getAxisValue(_input);
            float absValue = fabs(value);

            switch (_type) 
            {
            case Type::Hold:
                return absValue > (_durationThreshold * arg.scale);
            case Type::Press:
                return absValue > (_durationThreshold * arg.scale) &&
                    !(input.getPreviousAxisValue(_input) > (_durationThreshold * arg.scale));
                // ...其他轴特有逻辑...
            }
        }
        return false;
        }, _input);
}

void AYInputAction::rebind(UniversalInput input)
{
    _input = input;
}

const UniversalInput& AYInputAction::getInput() const
{
    return _input;
}