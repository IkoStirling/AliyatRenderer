#pragma once
#include "IAYModule.h"
#include "AYInputType.h"
#include "glm/glm.hpp"
namespace ayt::engine::input
{
    class AYInputBinding;

    struct InputState {
        bool current = false;           // 当前帧状态
        bool previous = false;          // 上一帧状态
        float duration = 0.0f;          // 按键持续时间(秒)
        int pressCount = 0;             // 连续按下次数(用于双击检测)
        float lastPressTime = 0.0f;     // 最后一次按下时间
        float value = 0.0f;             // 用于轴输入
        float previousValue = 0.0f;     // 上一帧轴值
        glm::vec2 pressPosition{ 0 };   // 对键盘输入无效
    };

    class Mod_InputSystem : public ayt::engine::modules::IModule
    {
    public:
        /*
        虚拟按键状态由此访问创建（唯一入口）
        */
        virtual InputState& getInputState(const UniversalInput& input) = 0;
        virtual const InputState* findInputState(const UniversalInput& input) const = 0;

        /*
            虚拟按键映射，这里提供映射中按键活动的状态访问方法
        */
        virtual void addInputMapping(const std::string& name, std::shared_ptr<AYInputBinding> binding) = 0;
        virtual void removeInputMapping(const std::string& name) = 0;
        virtual bool isActionActive(const std::string& bindingName, const std::string& actionName) const = 0;
        virtual bool isActionActive(const std::string& fullActionName) const = 0; //bindingName.actionName

        /*
            这里提供原始虚拟按键状态访问方法，！！！和上面的按键活动状态不同
        */
        virtual bool getUniversalInputState(const UniversalInput& input) const = 0;
        virtual bool getPreviousUniversalInputState(const UniversalInput& input) const = 0;

        /*
            轴值访问，如果当前按键不支持轴值则会返回 0
        */
        virtual float getAxisValue(const UniversalInput& input) const = 0;
        virtual float getScrollDelta(const UniversalInput& input) = 0;
        virtual float getPreviousAxisValue(const UniversalInput& input) const = 0;
        virtual glm::vec2 getVector2Axis(const std::string& fullActionName) const = 0;

        /*
            手柄相关
        */
        virtual void initGamepad(int joystickId) = 0;
        virtual bool isGamepadConnected(int joystickId) const = 0;
        virtual std::vector<int> getConnectedGamepads() const = 0;

        // 状态查询接口
        virtual bool isKeyPressed(int key) const = 0;
        virtual bool isKeyJustPressed(int key) const = 0;
        virtual bool isKeyReleased(int key) const = 0;
        virtual bool isKeyRepeated(int key) const = 0;

        virtual bool isMouseButtonDown(int button) const = 0;
        virtual bool isMouseButtonJustPressed(int button) const = 0;
        virtual bool isMouseButtonReleased(int button) const = 0;
        virtual glm::vec2 getMousePosition() const = 0;
        virtual glm::vec2 getMouseDelta() const = 0;
        virtual glm::vec2 getMouseButtonPressPosition(int button) const = 0;

        virtual int getKeyPressCount(int key) const = 0;
        virtual float getLastPressInterval(int key) const = 0;
        virtual float getKeyDuration(int key) const = 0;
        virtual float getMouseButtonDuration(int button) const = 0;

        virtual bool isShiftPressed() const = 0;
        virtual bool isCtrlPressed() const = 0;
        virtual bool isAltPressed() const = 0;

    };
}