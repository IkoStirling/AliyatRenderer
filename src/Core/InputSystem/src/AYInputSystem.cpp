#include "AYInputSystem.h"
#include "AYRendererManager.h"

AYInputSystem::AYInputSystem() :
	_currentMousePos(0.f),
	_lastMousePos(0.f)
{
}

AYInputSystem::~AYInputSystem()
{
	if (auto window = _device->getWindow()) {
		glfwSetKeyCallback(window, nullptr);
		glfwSetMouseButtonCallback(window, nullptr);
		glfwSetCursorPosCallback(window, nullptr);
		glfwSetScrollCallback(window, nullptr);
	}
}

void AYInputSystem::init()
{
	_device = GET_CAST_MODULE(Mod_Renderer, "Renderer")->getRenderDevice();
	if (!_device)
		throw std::runtime_error("InputSystem initialization failed\n");

	GLFWwindow* window = _device->getWindow();
	if (!window) return;

	// �����û�ָ���Ա��ھ�̬�ص��з���ʵ��
	//glfwSetWindowUserPointer(window, this); ���洰��ʵ��ȫ��Ψһ�������ڶദʹ����ɹ������

	// ����GLFW�ص�
	glfwSetKeyCallback(window, &AYInputSystem::keyCallbackWrapper);
	glfwSetMouseButtonCallback(window, &AYInputSystem::mouseButtonCallbackWrapper);
	glfwSetCursorPosCallback(window, &AYInputSystem::cursorPosCallbackWrapper);
	glfwSetScrollCallback(window, &AYInputSystem::scrollCallbackWrapper);

	initGamepad();
	for(int i = 0; i < GLFW_GAMEPAD_BUTTON_DPAD_LEFT + 1; i++)
		getInputState(GamepadButtonInput{ i });
}

void AYInputSystem::update(float delta_time)
{
	// ����״̬ת�ƴ���
	for (auto& [input, state] : _inputStates) {
		state.previous = state.current;
		state.previousValue = state.value;
	}

	_updateUniversalInputState(delta_time);
	_updateGamepadState(delta_time);

	// �׶�3���������ݼ���
	for (auto& [input, state] : _inputStates) {
		state.duration = state.current ? (state.duration + delta_time) : 0.0f;
	}

	_lastMousePos = _currentMousePos;
}

InputState& AYInputSystem::getInputState(const UniversalInput& input)
{
	auto& state = _inputStates[input];

	// ��ʼ��λ����Ϣ�������������룩
	if (std::holds_alternative<MouseButtonInput>(input))
	{
		if (state.pressPosition == glm::vec2(0))
		{
			state.pressPosition = _currentMousePos;
		}
	}
	return state;
}

const InputState* AYInputSystem::findInputState(const UniversalInput& input) const
{
	auto it = _inputStates.find(input);
	return it != _inputStates.end() ? &it->second : nullptr;
}

bool AYInputSystem::isKeyPressed(int key) const
{
	return getUniversalInputState(KeyboardInput{ key });
}

void AYInputSystem::addInputMapping(const std::string& name, std::shared_ptr<AYInputBinding> binding) {
	_keyBindings[name] = binding;
}

void AYInputSystem::removeInputMapping(const std::string& name) {
	_keyBindings.erase(name);
}

bool AYInputSystem::isActionActive(const std::string& bindingName, const std::string& actionName) const {
	auto it = _keyBindings.find(bindingName);
	if (it != _keyBindings.end()) {
		return it->second->isActive(actionName, *this);
	}
	return false;
}

bool AYInputSystem::isActionActive(const std::string& fullActionName) const {
	size_t dotPos = fullActionName.find('.');
	if (dotPos != std::string::npos) {
		std::string bindingName = fullActionName.substr(0, dotPos);
		std::string actionName = fullActionName.substr(dotPos + 1);
		return isActionActive(bindingName, actionName);
	}
	return false;
}

bool AYInputSystem::getUniversalInputState(const UniversalInput& input) const
{
	if (const InputState* state = findInputState(input)) {
		return state->current;
	}
	return false;
}

bool AYInputSystem::getPreviousUniversalInputState(const UniversalInput& input) const 
{
	if (const InputState* state = findInputState(input)) {
		return state->previous;
	}
	return false;
}

float AYInputSystem::getAxisValue(const UniversalInput& input) const
{
	return std::visit([this](auto&& arg) -> float {
		using T = std::decay_t<decltype(arg)>;

		if constexpr (std::is_same_v<T, MouseAxisInput>) {
			switch (arg.axis) {
			case MouseAxis::PositionX: return _currentMousePos.x * arg.scale;
			case MouseAxis::PositionY: return _currentMousePos.y * arg.scale;
			case MouseAxis::ScrollX:   return _scrollDelta.x * arg.scale;
			case MouseAxis::ScrollY:   return _scrollDelta.y * arg.scale;
			}
		}
		else if constexpr (std::is_same_v<T, GamepadAxisInput>) {
			if (_activeGamepad == -1) return 0.0f;

			float value = 0.0f;
			switch (arg.axis) {
			case GamepadAxis::LeftX:  value = _gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_X]; break;
			case GamepadAxis::LeftY:  value = _gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]; break;
			case GamepadAxis::RightX: value = _gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_X]; break;
			case GamepadAxis::RightY: value = _gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y]; break;
			case GamepadAxis::LeftTrigger:  value = _gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER]; break;
			case GamepadAxis::RightTrigger: value = _gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER]; break;
			}

			// Ӧ������������
			if (fabs(value) < arg.deadZone) return 0.0f;
			return value * arg.scale;
		}
		return 0.0f;
		}, input);
}

float AYInputSystem::getPreviousAxisValue(const UniversalInput& input) const
{
	auto it = _inputStates.find(input);
	if (it != _inputStates.end()) {
		return it->second.previousValue;
	}

	// ����δ��¼�������룬���ص�ǰֵ֡�������λ�ã�
	return std::visit([this](auto&& arg) -> float {
		using T = std::decay_t<decltype(arg)>;

		if constexpr (std::is_same_v<T, MouseAxisInput>) {
			switch (arg.axis) {
			case MouseAxis::PositionX: return _lastMousePos.x * arg.scale;
			case MouseAxis::PositionY: return _lastMousePos.y * arg.scale;
			case MouseAxis::ScrollX:   return 0; // ����û��"��һ֡"����
			case MouseAxis::ScrollY:   return 0;
			}
		}
		else if constexpr (std::is_same_v<T, GamepadAxisInput>) {
			return 0.0f; // ��Ϸ�ֱ���Ҫ���⴦��
		}
		return 0.0f;
		}, input);
}

glm::vec2 AYInputSystem::getVector2Axis(const std::string& fullActionName) const
{
	size_t dotPos = fullActionName.find('.');
	if (dotPos == std::string::npos)
		return glm::vec2();

	std::string bindingName = fullActionName.substr(0, dotPos);
	std::string actionName = fullActionName.substr(dotPos + 1);

	auto it = _keyBindings.find(bindingName);
	if (it == _keyBindings.end()) return glm::vec2(0);

	// ����X��Y�ᶯ��
	float x = 0.0f, y = 0.0f;
	bool hasX = false, hasY = false;

	for (const auto& [name, action] : it->second->getActions()) {
		if (name == actionName + "X") {
			x = getAxisValue(action.getInput());
			hasX = true;
		}
		else if (name == actionName + "Y") {
			y = getAxisValue(action.getInput());
			hasY = true;
		}
	}

	// ���û���ҵ��ض��󶨣�����ͨ�ò�ѯ
	if (!hasX || !hasY) {
		UniversalInput xInput = GamepadAxisInput{ GamepadAxis::LeftX };
		UniversalInput yInput = GamepadAxisInput{ GamepadAxis::LeftY };

		if (!hasX) x = getAxisValue(xInput);
		if (!hasY) y = getAxisValue(yInput);
	}

	return glm::vec2(x, -y); // ע��Y��ͨ����Ҫȡ��

}

void AYInputSystem::initGamepad(int joystickId)
{
	glfwSetJoystickCallback([](int jid, int event) {
		if (event == GLFW_CONNECTED)
		{

			std::cout << "�ֱ� " << jid << " ������" << std::endl;
		}
		else if (event == GLFW_DISCONNECTED)
		{
			std::cout << "�ֱ� " << jid << " �ѶϿ�" << std::endl;
		}
		});

	if (isGamepadConnected(joystickId))
	{
		_activeGamepad = joystickId;
	}
}

bool AYInputSystem::isGamepadConnected(int joystickId) const {
	double currentTime = glfwGetTime();

	// ÿ0.5��ˢ��һ��״̬�������Ƶ��ѯ��
	if (currentTime - _lastCheckTime > 0.5) {
		_lastCheckTime = currentTime;
		_gamepadStatusCache.clear();

		for (int i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; ++i) {
			_gamepadStatusCache[i] =
				glfwJoystickPresent(i) &&
				glfwJoystickIsGamepad(i);
		}
	}

	return _gamepadStatusCache.count(joystickId) ? _gamepadStatusCache.at(joystickId) : false;
}

std::vector<int> AYInputSystem::getConnectedGamepads() const
{
	std::vector<int> result;
	for (int jid = GLFW_JOYSTICK_1; jid <= GLFW_JOYSTICK_LAST; ++jid)
	{
		if (isGamepadConnected(jid)) {
			result.push_back(jid);
		}
	}
	return result;
}

bool AYInputSystem::isKeyJustPressed(int key) const
{
	auto helper = KeyboardInput{ key };
	return getUniversalInputState(helper) && !getPreviousUniversalInputState(helper);
}

bool AYInputSystem::isKeyReleased(int key) const
{
	auto helper = KeyboardInput{ key };
	return !getUniversalInputState(helper) && getPreviousUniversalInputState(helper);
}

bool AYInputSystem::isKeyRepeated(int key) const
{
	auto helper = KeyboardInput{ key };
	return getUniversalInputState(helper) && getPreviousUniversalInputState(helper);
}

bool AYInputSystem::isMouseButtonDown(int button) const
{
	return getUniversalInputState(MouseButtonInput{ button });
}

bool AYInputSystem::isMouseButtonJustPressed(int button) const 
{
	auto helper = MouseButtonInput{ button };
	return getUniversalInputState(helper) && !getPreviousUniversalInputState(helper);
}

bool AYInputSystem::isMouseButtonReleased(int button) const 
{
	auto helper = MouseButtonInput{ button };
	return !getUniversalInputState(helper) && getPreviousUniversalInputState(helper);
}

glm::vec2 AYInputSystem::getMousePosition() const 
{
	return _currentMousePos;
}

glm::vec2 AYInputSystem::getMouseDelta() const 
{
	return _currentMousePos - _lastMousePos;
}

glm::vec2 AYInputSystem::getMouseButtonPressPosition(int button) const 
{
	// û��ʵ��
	auto helper = MouseButtonInput{ button };

	return glm::vec2(0.0f);
}

// �߼����빦��
int AYInputSystem::getKeyPressCount(int key) const 
{
	auto helper = KeyboardInput{ key };
	const InputState* state = findInputState(helper);
	return state ? state->pressCount : 0;
}

float AYInputSystem::getLastPressInterval(int key) const
{
	auto helper = KeyboardInput{ key };
	const InputState* state = findInputState(helper);
	return state ? (float)glfwGetTime() - state->lastPressTime : FLT_MAX;
}

float AYInputSystem::getKeyDuration(int key) const 
{
	auto helper = KeyboardInput{ key };
	const InputState* state = findInputState(helper);
	return state ? state->duration : 0.f;
}

float AYInputSystem::getMouseButtonDuration(int button) const 
{
	auto helper = MouseButtonInput{ button };
	const InputState* state = findInputState(helper);
	return state ? state->duration : 0.f;
}

// �������μ�
bool AYInputSystem::isShiftPressed() const 
{
	return glfwGetKey(_device->getWindow(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
		glfwGetKey(_device->getWindow(), GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
}

bool AYInputSystem::isCtrlPressed() const 
{
	return glfwGetKey(_device->getWindow(), GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
		glfwGetKey(_device->getWindow(), GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
}

bool AYInputSystem::isAltPressed() const
{
	return glfwGetKey(_device->getWindow(), GLFW_KEY_LEFT_ALT) == GLFW_PRESS ||
		glfwGetKey(_device->getWindow(), GLFW_KEY_RIGHT_ALT) == GLFW_PRESS;
}





void AYInputSystem::_updateUniversalInputState(float delta_time)
{

}

void AYInputSystem::_updateAxisStates(float delta_time)
{
	// �������λ����
	getInputState(MouseAxisInput{ MouseAxis::PositionX }).value = _currentMousePos.x;
	getInputState(MouseAxisInput{ MouseAxis::PositionY }).value = _currentMousePos.y;

	// �����ֱ���
	if (_activeGamepad != -1) {
		auto updateGamepadAxis = [&](GamepadAxis axis, int glfwAxis) {
			auto& state = getInputState(GamepadAxisInput{ axis });
			state.value = _gamepadState.axes[glfwAxis];
			};

		updateGamepadAxis(GamepadAxis::LeftX, GLFW_GAMEPAD_AXIS_LEFT_X);
		updateGamepadAxis(GamepadAxis::LeftY, GLFW_GAMEPAD_AXIS_LEFT_Y);
		updateGamepadAxis(GamepadAxis::RightX, GLFW_GAMEPAD_AXIS_RIGHT_X);
		updateGamepadAxis(GamepadAxis::RightY, GLFW_GAMEPAD_AXIS_RIGHT_X);
		updateGamepadAxis(GamepadAxis::LeftTrigger, GLFW_GAMEPAD_AXIS_LEFT_TRIGGER);
		updateGamepadAxis(GamepadAxis::RightTrigger, GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER);
	}
}

void AYInputSystem::_updateGamepadState(float delta_time)
{
	if (!isGamepadConnected(_activeGamepad)) 
	{
		_activeGamepad = -1; // ��ǶϿ�
		auto gamepads = getConnectedGamepads();
		if (!gamepads.empty())
			_activeGamepad = gamepads[0];
		else
			return;
	}
	if (_activeGamepad != -1 && glfwGetGamepadState(_activeGamepad, &_gamepadState)) 
	{
		// ����������ע����ֱ���ť״̬, �����¹�ȥ״̬
		for (auto& [input, state] : _inputStates) {
			if (auto btn = std::get_if<GamepadButtonInput>(&input))
			{
				bool pressed = _gamepadState.buttons[btn->button] == GLFW_PRESS;
				if (pressed != state.current) {
					state.current = pressed;
					if (pressed) {
						state.pressCount++;
						state.lastPressTime = (float)glfwGetTime();
					}
				}
			}
		}
	}
}

















void AYInputSystem::handleKey(int key, int scancode, int action, int mods) 
{
	UniversalInput input = KeyboardInput{ key };
	auto& state = getInputState(input);

	if (action == GLFW_PRESS)
	{
		state.current = true;
		state.pressCount++;  
		state.lastPressTime = (float)glfwGetTime();
	}
	else if (action == GLFW_RELEASE) 
	{
		state.current = false;
	}
}

void AYInputSystem::handleMouseButton(int button, int action, int mods) 
{
	UniversalInput input = MouseButtonInput{ button };
	auto& state = getInputState(input);

	if (action == GLFW_PRESS) 
	{
		state.current = true;
		state.pressCount++;  // ��¼���´���������˫����⣩
		state.lastPressTime = (float)glfwGetTime();
		state.pressPosition = _currentMousePos; // ��¼����ʱ�����λ��
	}
	else if (action == GLFW_RELEASE) 
	{
		state.current = false;
	}
}

void AYInputSystem::handleMousePosition(double x, double y) 
{
	_currentMousePos = glm::vec2((float)x, (float)y);
}

void AYInputSystem::handleScroll(double xoffset, double yoffset) 
{
	_scrollDelta = glm::vec2(xoffset, yoffset);

	// ������״̬
	UniversalInput xInput{ MouseAxisInput{MouseAxis::ScrollX} };
	UniversalInput yInput{ MouseAxisInput{MouseAxis::ScrollY} };

	getInputState(xInput).value = xoffset;
	getInputState(yInput).value = yoffset;
}

void AYInputSystem::keyCallbackWrapper(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	auto inputSystem = GET_CAST_MODULE(AYInputSystem, "InputSystem");
	if (inputSystem) inputSystem->handleKey(key, scancode, action, mods);
}

void AYInputSystem::mouseButtonCallbackWrapper(GLFWwindow* window, int button, int action, int mods) 
{
	auto inputSystem = GET_CAST_MODULE(AYInputSystem, "InputSystem");
	if (inputSystem) inputSystem->handleMouseButton(button, action, mods);
}

void AYInputSystem::cursorPosCallbackWrapper(GLFWwindow* window, double xpos, double ypos) 
{
	auto inputSystem = GET_CAST_MODULE(AYInputSystem, "InputSystem");
	if (inputSystem) inputSystem->handleMousePosition(xpos, ypos);
}

void AYInputSystem::scrollCallbackWrapper(GLFWwindow* window, double xoffset, double yoffset) 
{
	auto inputSystem = GET_CAST_MODULE(AYInputSystem, "InputSystem");
	if (inputSystem) inputSystem->handleScroll(xoffset, yoffset);
}