#pragma once
#include "Mod_InputSystem.h"
#include "AYRenderDevice.h"
#include "AYInputBinding.h"
#include "glm/glm.hpp"

namespace ayt::engine::input
{
    using ayt::engine::render::RenderDevice;

    class InputSystem : public Mod_InputSystem
    {
    public:
        InputSystem();
        ~InputSystem();

        void init() override;

        void update(float delta_time) override;

        void shutdown() override;
        /*
            虚拟按键状态由此访问创建（唯一入口）
            在回调内已经自动创建了键盘和鼠标的虚拟按键状态
            但是手柄并没有创建，需要手动初始化
        */
        InputState& getInputState(const UniversalInput& input);
        const InputState* findInputState(const UniversalInput& input) const;

        /*
            虚拟按键映射，这里提供映射中按键活动的状态访问方法
        */
        void addInputMapping(const std::string& name, std::shared_ptr<InputBinding> binding);
        void removeInputMapping(const std::string& name);
        bool isActionActive(const std::string& bindingName, const std::string& actionName) const;
        bool isActionActive(const std::string& fullActionName) const; //bindingName.actionName
        float getLongPressHoldTime(const std::string& bindingName, const std::string& actionName) const;
        float getLongPressHoldTime(const std::string& fullActionName) const;
        bool isActionJustReleased(const std::string& bindingName, const std::string& actionName) const;
        bool isActionJustReleased(const std::string& fullActionName) const;

        /*
            这里提供原始虚拟按键状态访问方法，！！！和上面的按键活动状态不同
            但是如果不getInputState一次，就不会生效
        */
        float getUniversalDuration(const UniversalInput& input) const;
        bool getUniversalInputState(const UniversalInput& input) const;
        bool getPreviousUniversalInputState(const UniversalInput& input) const;

        /*
            轴值访问，如果当前按键不支持轴值则会返回 0
        */
        float getAxisValue(const UniversalInput& input) const;
        float getScrollDelta(const UniversalInput& input);
        float getPreviousAxisValue(const UniversalInput& input) const;
        glm::vec2 getVector2Axis(const std::string& fullActionName) const;

        /*
            手柄相关
        */
        void initGamepad(int joystickId = GLFW_JOYSTICK_1);
        bool isGamepadConnected(int joystickId = GLFW_JOYSTICK_1) const;
        std::vector<int> getConnectedGamepads() const;

    private:

        // 输入逻辑更新
        void _updateUniversalInputState(float delta_time);
        void _updateAxisStates(float delta_time);   //轴值实时访问，该函数暂时弃用
        void _updateGamepadState(float delta_time);

        void _debugInfo()
        {
            for (auto& [input, state] : _inputStates)
            {
                if (auto btn = std::get_if<GamepadButtonInput>(&input))
                {
                    spdlog::debug("[InputSystem] current: {}, previous: {}", state.current, state.previous);
                }
            }
        }
    private:
        RenderDevice* _device = nullptr;

        // 按键映射存储
        std::unordered_map<std::string, std::shared_ptr<InputBinding>> _keyBindings;

        // 输入状态存储
        std::unordered_map<
            UniversalInput,
            InputState,
            UniversalInputHash,
            UniversalInputEqual
        > _inputStates;

        glm::vec2 _currentMousePos = glm::vec2(0);     //通过回调获取
        glm::vec2 _lastMousePos = glm::vec2(0);        //通过逻辑更新获取
        glm::vec2 _scrollDelta = glm::vec2(0);         //通过回调获取
        int _activeGamepad = -1;
        GLFWgamepadstate _gamepadState; //通过逻辑更新获取
        mutable std::unordered_map<int, bool> _gamepadStatusCache;  //gamepad缓存
        mutable double _lastCheckTime = 0.0;

    public:
        /*
        按键信息获取接口（old）
        */
        bool isKeyPressed(int key) const override;
        bool isKeyJustPressed(int key) const override;
        bool isKeyReleased(int key) const override;
        bool isKeyRepeated(int key) const override;

        bool isMouseButtonDown(int button) const override;
        bool isMouseButtonJustPressed(int button) const override;
        bool isMouseButtonReleased(int button) const override;
        glm::vec2 getMousePosition() const override;
        glm::vec2 getMouseDelta() const override;
        glm::vec2 getMouseButtonPressPosition(int button) const override;

        int getKeyPressCount(int key) const override;
        float getLastPressInterval(int key) const override;
        float getKeyDuration(int key) const override;
        float getMouseButtonDuration(int button) const override;

        bool isShiftPressed() const override;
        bool isCtrlPressed() const override;
        bool isAltPressed() const override;
    private:
        // 内部回调处理
        void handleKey(int key, int scancode, int action, int mods);    //这里更新虚拟按键状态（如没有会自动添加）
        void handleMouseButton(int button, int action, int mods);
        void handleMousePosition(double x, double y);
        void handleScroll(double xoffset, double yoffset);

        // 静态回调转发器
        static void keyCallbackWrapper(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void mouseButtonCallbackWrapper(GLFWwindow* window, int button, int action, int mods);
        static void cursorPosCallbackWrapper(GLFWwindow* window, double xpos, double ypos);
        static void scrollCallbackWrapper(GLFWwindow* window, double xoffset, double yoffset);
    };

    REGISTER_MODULE_CLASS("InputSystem", InputSystem)
}