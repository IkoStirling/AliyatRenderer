#pragma once
#include "Mod_InputSystem.h"
#include "AYRenderDevice.h"
#include "AYInputBinding.h"
#include "glm/glm.hpp"


class AYInputSystem : public Mod_InputSystem
{
public:
	AYInputSystem();
    ~AYInputSystem();

	void init() override;

	void update(float delta_time) override;

    /*
        ���ⰴ��״̬�ɴ˷��ʴ�����Ψһ��ڣ�
        �ڻص����Ѿ��Զ������˼��̺��������ⰴ��״̬
        �����ֱ���û�д�������Ҫ�ֶ���ʼ��
    */
    InputState& getInputState(const UniversalInput& input); 
    const InputState* findInputState(const UniversalInput& input) const;

    /*
        ���ⰴ��ӳ�䣬�����ṩӳ���а������״̬���ʷ���
    */
    void addInputMapping(const std::string& name, std::shared_ptr<AYInputBinding> binding);
    void removeInputMapping(const std::string& name);
    bool isActionActive(const std::string& bindingName, const std::string& actionName) const;
    bool isActionActive(const std::string& fullActionName) const; //bindingName.actionName

    /*
        �����ṩԭʼ���ⰴ��״̬���ʷ�����������������İ����״̬��ͬ
        ���������getInputStateһ�Σ��Ͳ�����Ч
    */
    bool getUniversalInputState(const UniversalInput& input) const;
    bool getPreviousUniversalInputState(const UniversalInput& input) const;

    /*
        ��ֵ���ʣ������ǰ������֧����ֵ��᷵�� 0
    */
    float getAxisValue(const UniversalInput& input) const;
    float getPreviousAxisValue(const UniversalInput& input) const;
    glm::vec2 getVector2Axis(const std::string& fullActionName) const;
    
    /*
        �ֱ����
    */
    void initGamepad(int joystickId = GLFW_JOYSTICK_1);
    bool isGamepadConnected(int joystickId = GLFW_JOYSTICK_1) const;
    std::vector<int> getConnectedGamepads() const;

private:
    // �����߼�����
    void _updateUniversalInputState(float delta_time);
    void _updateAxisStates(float delta_time);   //��ֵʵʱ���ʣ��ú�����ʱ����
    void _updateGamepadState(float delta_time);

    void _debugInfo()
    {
        for(auto & [input, state] : _inputStates)
        {
            if (auto btn = std::get_if<GamepadButtonInput>(&input))
            {
                std::cout << "current: " << state.current << std::endl;
                std::cout << "previous: " << state.previous << std::endl << std::endl;
            }
        }
    }
private:
    AYRenderDevice* _device = nullptr;

    // ����ӳ��洢
    std::unordered_map<std::string, std::shared_ptr<AYInputBinding>> _keyBindings;

    // ����״̬�洢
    std::unordered_map<
        UniversalInput,
        InputState,
        UniversalInputHash,
        UniversalInputEqual
    > _inputStates;

    glm::vec2 _currentMousePos;     //ͨ���ص���ȡ
    glm::vec2 _lastMousePos;        //ͨ���߼����»�ȡ
    glm::vec2 _scrollDelta;         //ͨ���ص���ȡ
    int _activeGamepad = -1;
    GLFWgamepadstate _gamepadState; //ͨ���߼����»�ȡ
    mutable std::unordered_map<int, bool> _gamepadStatusCache;  //gamepad����
    mutable double _lastCheckTime = 0.0;

public:
    /*
    ������Ϣ��ȡ�ӿڣ�old��
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
    // �ڲ��ص�����
    void handleKey(int key, int scancode, int action, int mods);    //����������ⰴ��״̬����û�л��Զ���ӣ�
    void handleMouseButton(int button, int action, int mods);
    void handleMousePosition(double x, double y);
    void handleScroll(double xoffset, double yoffset); 

    // ��̬�ص�ת����
    static void keyCallbackWrapper(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallbackWrapper(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallbackWrapper(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallbackWrapper(GLFWwindow* window, double xoffset, double yoffset);
};

REGISTER_MODULE_CLASS("InputSystem", AYInputSystem)
