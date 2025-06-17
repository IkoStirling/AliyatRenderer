#pragma once
#include <glm/glm.hpp>
#include <variant>

enum class InputDevice
{
    Keyboard,
    Mouse,
    Gamepad
};

enum class MouseAxis 
{
    PositionX,  // 鼠标X坐标
    PositionY,  // 鼠标Y坐标
    ScrollX,    // 滚轮水平滚动
    ScrollY     // 滚轮垂直滚动
};

// 手柄轴类型 (兼容XInput和标准手柄)
enum class GamepadAxis 
{
    LeftX,      // 左摇杆X轴
    LeftY,      // 左摇杆Y轴
    RightX,     // 右摇杆X轴
    RightY,     // 右摇杆Y轴
    LeftTrigger, // 左扳机
    RightTrigger // 右扳机
};

struct KeyboardInput 
{
    int key; // GLFW_KEY_*
};

struct MouseButtonInput 
{
    int button; // GLFW_MOUSE_BUTTON_*
};

struct MouseAxisInput
{
    MouseAxis axis;
    float scale = 1.0f;
};

struct GamepadButtonInput
{
    int button; // GLFW_JOYSTICK_BUTTON_*
};

struct GamepadAxisInput
{
                        // GLFW_JOYSTICK_AXIS_*
    GamepadAxis axis;
    float scale = 1.0f;
    float deadZone = 0.2f;
};

using UniversalInput = std::variant<
    KeyboardInput,
    MouseAxisInput,
    MouseButtonInput,
    GamepadButtonInput,
    GamepadAxisInput
>;

#include <functional>

// 哈希函数定义
struct UniversalInputHash {
    size_t operator()(const UniversalInput& input) const {
        return std::visit([](auto&& arg) -> size_t {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, KeyboardInput>) {
                return std::hash<int>()(arg.key) ^ 0x9e3779b9;
            }
            else if constexpr (std::is_same_v<T, MouseAxisInput>) {
                return std::hash<int>()((int)arg.axis) ^ 0x7e9eab22;
            }
            else if constexpr (std::is_same_v<T, MouseButtonInput>) {
                return std::hash<int>()(arg.button) ^ 0x3c6ef372;
            }
            else if constexpr (std::is_same_v<T, GamepadButtonInput>) {
                return std::hash<int>()(arg.button) ^ 0xdaa66d2b;
            }
            else if constexpr (std::is_same_v<T, GamepadAxisInput>) {
                return std::hash<int>()((int)arg.axis) ^
                    std::hash<float>()(arg.deadZone) ^ 0x6a09e667;
            }
            return 0;
            }, input);
    }
};

// 相等比较函数
struct UniversalInputEqual {
    bool operator()(const UniversalInput& a, const UniversalInput& b) const {
        if (a.index() != b.index()) return false;
        return std::visit([](auto&& argA, auto&& argB) -> bool {
            using TA = std::decay_t<decltype(argA)>;
            using TB = std::decay_t<decltype(argB)>;
            if constexpr (std::is_same_v<TA, TB>) {
                if constexpr (std::is_same_v<TA, KeyboardInput>) {
                    return argA.key == argB.key;
                }
                else if constexpr (std::is_same_v<TA, MouseAxisInput>) {
                    return argA.axis == argB.axis;
                }
                else if constexpr (std::is_same_v<TA, MouseButtonInput>) {
                    return argA.button == argB.button;
                }
                else if constexpr (std::is_same_v<TA, GamepadButtonInput>) {
                    return argA.button == argB.button;
                }
                else if constexpr (std::is_same_v<TA, GamepadAxisInput>) {
                    return argA.axis == argB.axis &&
                        abs(argA.deadZone - argB.deadZone) < FLT_EPSILON;
                }
            }
            return false;
            }, a, b);
    }
};