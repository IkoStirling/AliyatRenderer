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
    PositionX,  // ���X����
    PositionY,  // ���Y����
    ScrollX,    // ����ˮƽ����
    ScrollY     // ���ִ�ֱ����
};

// �ֱ������� (����XInput�ͱ�׼�ֱ�)
enum class GamepadAxis 
{
    LeftX,      // ��ҡ��X��
    LeftY,      // ��ҡ��Y��
    RightX,     // ��ҡ��X��
    RightY,     // ��ҡ��Y��
    LeftTrigger, // ����
    RightTrigger // �Ұ��
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

// ��ϣ��������
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

// ��ȱȽϺ���
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