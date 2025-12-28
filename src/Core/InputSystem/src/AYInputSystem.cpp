#include "AYInputSystem.h"
#include "AYRendererManager.h"
namespace ayt::engine::input
{
	using namespace ::ayt::engine::event;
	using ayt::engine::render::Mod_Renderer;

	InputSystem::InputSystem() :
		_currentMousePos(0.f),
		_lastMousePos(0.f)
	{
	}

	InputSystem::~InputSystem()
	{
		if (auto window = _device->getWindow()) {
			glfwSetKeyCallback(window, nullptr);
			glfwSetMouseButtonCallback(window, nullptr);
			glfwSetCursorPosCallback(window, nullptr);
			glfwSetScrollCallback(window, nullptr);
		}
	}

	void InputSystem::init()
	{
		_device = GET_CAST_MODULE(Mod_Renderer, "Renderer")->getRenderDevice();
		if (!_device)
			throw std::runtime_error("InputSystem initialization failed\n");

		GLFWwindow* window = _device->getWindow();
		if (!window) return;

		// 设置用户指针以便在静态回调中访问实例
		//glfwSetWindowUserPointer(window, this); 跟随窗口实例全局唯一，可能在多处使用造成管理混乱

		// 设置GLFW回调
		glfwSetKeyCallback(window, &InputSystem::keyCallbackWrapper);
		glfwSetMouseButtonCallback(window, &InputSystem::mouseButtonCallbackWrapper);
		glfwSetCursorPosCallback(window, &InputSystem::cursorPosCallbackWrapper);
		glfwSetScrollCallback(window, &InputSystem::scrollCallbackWrapper);

		initGamepad();
		for (int i = 0; i < GLFW_GAMEPAD_BUTTON_DPAD_LEFT + 1; i++)
			getInputState(GamepadButtonInput{ i });
	}

	void InputSystem::update(float delta_time)
	{
		_lastMousePos = _currentMousePos;

		// 所有状态转移处理
		for (auto& [input, state] : _inputStates) {
			state.previous = state.current;
			state.previousValue = state.value;
		}

		_updateUniversalInputState(delta_time);
		_updateGamepadState(delta_time);

		// 阶段3：衍生数据计算
		for (auto& [input, state] : _inputStates) {
			state.duration = state.current ? (state.duration + delta_time) : 0.0f;
		}

	}

	void InputSystem::shutdown()
	{
		std::unordered_map<
			UniversalInput,
			InputState,
			UniversalInputHash,
			UniversalInputEqual
		> is;
		std::unordered_map<std::string, std::shared_ptr<InputBinding>> kb;
		std::unordered_map<int, bool> gs;

		kb.swap(_keyBindings);
		is.swap(_inputStates);
		gs.swap(_gamepadStatusCache);
	}

	InputState& InputSystem::getInputState(const UniversalInput& input)
	{
		auto& state = _inputStates[input];

		// 初始化位置信息（如果是鼠标输入）
		if (std::holds_alternative<MouseButtonInput>(input))
		{
			if (state.pressPosition == glm::vec2(0))
			{
				state.pressPosition = _currentMousePos;
			}
		}
		return state;
	}

	const InputState* InputSystem::findInputState(const UniversalInput& input) const
	{
		auto it = _inputStates.find(input);
		return it != _inputStates.end() ? &it->second : nullptr;
	}

	bool InputSystem::isKeyPressed(int key) const
	{
		return getUniversalInputState(KeyboardInput{ key });
	}

	void InputSystem::addInputMapping(const std::string& name, std::shared_ptr<InputBinding> binding) {
		_keyBindings[name] = binding;
	}

	void InputSystem::removeInputMapping(const std::string& name) {
		_keyBindings.erase(name);
	}

	bool InputSystem::isActionActive(const std::string& bindingName, const std::string& actionName) const {
		auto it = _keyBindings.find(bindingName);
		if (it != _keyBindings.end()) {
			return it->second->isActive(actionName, *this);
		}
		return false;
	}

	bool InputSystem::isActionActive(const std::string& fullActionName) const
	{
		if (size_t dotPos = fullActionName.find('.'); dotPos != std::string::npos) {
			std::string bindingName = fullActionName.substr(0, dotPos);
			std::string actionName = fullActionName.substr(dotPos + 1);
			return isActionActive(bindingName, actionName);
		}
		return false;
	}

	float InputSystem::getLongPressHoldTime(const std::string& bindingName, const std::string& actionName) const
	{
		if (auto it = _keyBindings.find(bindingName); it != _keyBindings.end()) {
			if (!it->second->hasAction(actionName))
				return 0.0f;
			const auto& input = it->second->getAction(actionName).getInput();
			return getUniversalDuration(input);
		}
		return 0.0f;
	}

	float InputSystem::getLongPressHoldTime(const std::string& fullActionName) const
	{
		if (size_t dotPos = fullActionName.find('.'); dotPos != std::string::npos) {
			std::string bindingName = fullActionName.substr(0, dotPos);
			std::string actionName = fullActionName.substr(dotPos + 1);
			return getLongPressHoldTime(bindingName, actionName);
		}
		return 0.0f;
	}

	bool InputSystem::isActionJustReleased(const std::string& bindingName, const std::string& actionName) const
	{
		if (auto it = _keyBindings.find(bindingName); it != _keyBindings.end()) {
			if (!it->second->hasAction(actionName))
				return false;
			const auto& input = it->second->getAction(actionName).getInput();
			return !getUniversalInputState(input) && getPreviousUniversalInputState(input);
		}
		return false;
	}

	bool InputSystem::isActionJustReleased(const std::string& fullActionName) const
	{
		if (size_t dotPos = fullActionName.find('.'); dotPos != std::string::npos) {
			std::string bindingName = fullActionName.substr(0, dotPos);
			std::string actionName = fullActionName.substr(dotPos + 1);
			return isActionJustReleased(bindingName, actionName);
		}
		return false;
	}

	float InputSystem::getUniversalDuration(const UniversalInput& input) const
	{
		if (const InputState* state = findInputState(input)) {
			return state->duration;
		}
		return 0.0f;
	}

	bool InputSystem::getUniversalInputState(const UniversalInput& input) const
	{
		if (const InputState* state = findInputState(input)) {
			return state->current;
		}
		return false;
	}

	bool InputSystem::getPreviousUniversalInputState(const UniversalInput& input) const
	{
		if (const InputState* state = findInputState(input)) {
			return state->previous;
		}
		return false;
	}

	float InputSystem::getAxisValue(const UniversalInput& input) const
	{
		return std::visit([this](auto&& arg) -> float {
			using T = std::decay_t<decltype(arg)>;

			if constexpr (std::is_same_v<T, MouseAxisInput>) {
				switch (arg.axis) {
				case MouseAxis::PositionX: return _currentMousePos.x * arg.scale;
				case MouseAxis::PositionY: return _currentMousePos.y * arg.scale;
				case MouseAxis::ScrollX:
				case MouseAxis::ScrollY:
					throw std::logic_error("Scroll axis should be consumed via getScrollDelta()!");
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

				// 应用死区和缩放
				if (fabs(value) < arg.deadZone) return 0.0f;
				return value * arg.scale;
			}
			return 0.0f;
			}, input);
	}

	float InputSystem::getScrollDelta(const UniversalInput& input)
	{
		return std::visit([this](auto&& arg) -> float {
			using T = std::decay_t<decltype(arg)>;

			if constexpr (std::is_same_v<T, MouseAxisInput>) {
				float tmp;
				switch (arg.axis) {
				case MouseAxis::ScrollX:
					tmp = _scrollDelta.x;
					_scrollDelta = glm::vec2(0);
					return tmp;
				case MouseAxis::ScrollY:
					tmp = _scrollDelta.y;
					_scrollDelta = glm::vec2(0);
					return tmp;
				}
			}
			return 0.0f;
			}, input);
	}

	float InputSystem::getPreviousAxisValue(const UniversalInput& input) const
	{
		auto it = _inputStates.find(input);
		if (it != _inputStates.end()) {
			return it->second.previousValue;
		}

		// 对于未记录的轴输入，返回当前帧值（如鼠标位置）
		return std::visit([this](auto&& arg) -> float {
			using T = std::decay_t<decltype(arg)>;

			if constexpr (std::is_same_v<T, MouseAxisInput>) {
				switch (arg.axis) {
				case MouseAxis::PositionX: return _lastMousePos.x * arg.scale;
				case MouseAxis::PositionY: return _lastMousePos.y * arg.scale;
				case MouseAxis::ScrollX:   return 0; // 滚轮没有"上一帧"概念
				case MouseAxis::ScrollY:   return 0;
				}
			}
			else if constexpr (std::is_same_v<T, GamepadAxisInput>) {
				return 0.0f; // 游戏手柄需要特殊处理
			}
			return 0.0f;
			}, input);
	}

	glm::vec2 InputSystem::getVector2Axis(const std::string& fullActionName) const
	{
		size_t dotPos = fullActionName.find('.');
		if (dotPos == std::string::npos)
			return glm::vec2();

		std::string bindingName = fullActionName.substr(0, dotPos);
		std::string actionName = fullActionName.substr(dotPos + 1);

		auto it = _keyBindings.find(bindingName);
		if (it == _keyBindings.end()) return glm::vec2(0);

		// 查找X和Y轴动作
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

		// 如果没有找到特定绑定，尝试通用查询
		if (!hasX || !hasY) {
			UniversalInput xInput = GamepadAxisInput{ GamepadAxis::LeftX };
			UniversalInput yInput = GamepadAxisInput{ GamepadAxis::LeftY };

			if (!hasX) x = getAxisValue(xInput);
			if (!hasY) y = getAxisValue(yInput);
		}

		return glm::vec2(x, -y); // 注意Y轴通常需要取反

	}

	void InputSystem::initGamepad(int joystickId)
	{
		glfwSetJoystickCallback([](int jid, int event) {
			if (event == GLFW_CONNECTED)
			{
				AYLOG_INFO("[InputSystem] joystick {} connected", jid);
			}
			else if (event == GLFW_DISCONNECTED)
			{
				AYLOG_INFO("[InputSystem] joystick {} disconnected", jid);
			}
			});

		if (isGamepadConnected(joystickId))
		{
			_activeGamepad = joystickId;
		}
	}

	bool InputSystem::isGamepadConnected(int joystickId) const {
		double currentTime = glfwGetTime();

		// 每0.5秒刷新一次状态（避免高频查询）
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

	std::vector<int> InputSystem::getConnectedGamepads() const
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

	bool InputSystem::isKeyJustPressed(int key) const
	{
		auto helper = KeyboardInput{ key };
		return getUniversalInputState(helper) && !getPreviousUniversalInputState(helper);
	}

	bool InputSystem::isKeyReleased(int key) const
	{
		auto helper = KeyboardInput{ key };
		return !getUniversalInputState(helper) && getPreviousUniversalInputState(helper);
	}

	bool InputSystem::isKeyRepeated(int key) const
	{
		auto helper = KeyboardInput{ key };
		return getUniversalInputState(helper) && getPreviousUniversalInputState(helper);
	}

	bool InputSystem::isMouseButtonDown(int button) const
	{
		return getUniversalInputState(MouseButtonInput{ button });
	}

	bool InputSystem::isMouseButtonJustPressed(int button) const
	{
		auto helper = MouseButtonInput{ button };
		return getUniversalInputState(helper) && !getPreviousUniversalInputState(helper);
	}

	bool InputSystem::isMouseButtonReleased(int button) const
	{
		auto helper = MouseButtonInput{ button };
		return !getUniversalInputState(helper) && getPreviousUniversalInputState(helper);
	}

	glm::vec2 InputSystem::getMousePosition() const
	{
		return _currentMousePos;
	}

	glm::vec2 InputSystem::getMouseDelta() const
	{
		return _currentMousePos - _lastMousePos;
	}

	glm::vec2 InputSystem::getMouseButtonPressPosition(int button) const
	{
		// 没有实现
		auto helper = MouseButtonInput{ button };

		return glm::vec2(0.0f);
	}

	// 高级输入功能
	int InputSystem::getKeyPressCount(int key) const
	{
		auto helper = KeyboardInput{ key };
		const InputState* state = findInputState(helper);
		return state ? state->pressCount : 0;
	}

	float InputSystem::getLastPressInterval(int key) const
	{
		auto helper = KeyboardInput{ key };
		const InputState* state = findInputState(helper);
		return state ? (float)glfwGetTime() - state->lastPressTime : FLT_MAX;
	}

	float InputSystem::getKeyDuration(int key) const
	{
		auto helper = KeyboardInput{ key };
		const InputState* state = findInputState(helper);
		return state ? state->duration : 0.f;
	}

	float InputSystem::getMouseButtonDuration(int button) const
	{
		auto helper = MouseButtonInput{ button };
		const InputState* state = findInputState(helper);
		return state ? state->duration : 0.f;
	}

	// 输入修饰键
	bool InputSystem::isShiftPressed() const
	{
		return glfwGetKey(_device->getWindow(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
			glfwGetKey(_device->getWindow(), GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
	}

	bool InputSystem::isCtrlPressed() const
	{
		return glfwGetKey(_device->getWindow(), GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
			glfwGetKey(_device->getWindow(), GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
	}

	bool InputSystem::isAltPressed() const
	{
		return glfwGetKey(_device->getWindow(), GLFW_KEY_LEFT_ALT) == GLFW_PRESS ||
			glfwGetKey(_device->getWindow(), GLFW_KEY_RIGHT_ALT) == GLFW_PRESS;
	}

	void InputSystem::_updateUniversalInputState(float delta_time)
	{
		GLFWwindow* window = _device->getWindow();
		if (!window)
			return;

		// 遍历所有注册的输入，主动查询当前硬件状态并更新 state.current
		for (auto& [input, state] : _inputStates)
		{
			if (auto keyInput = std::get_if<KeyboardInput>(&input))
			{
				int key = keyInput->key;
				state.current = glfwGetKey(window, key) == GLFW_PRESS;
			}
			else if (auto mouseBtnInput = std::get_if<MouseButtonInput>(&input))
			{
				int button = mouseBtnInput->button;
				bool mstate = glfwGetMouseButton(window, button) == GLFW_PRESS;
				state.current = mstate;
			}
		}
	}

	void InputSystem::_updateAxisStates(float delta_time)
	{
		// 更新鼠标位置轴
		getInputState(MouseAxisInput{ MouseAxis::PositionX }).value = _currentMousePos.x;
		getInputState(MouseAxisInput{ MouseAxis::PositionY }).value = _currentMousePos.y;

		// 更新手柄轴
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

	void InputSystem::_updateGamepadState(float delta_time)
	{
		if (!isGamepadConnected(_activeGamepad))
		{
			_activeGamepad = -1; // 标记断开
			auto gamepads = getConnectedGamepads();
			if (!gamepads.empty())
				_activeGamepad = gamepads[0];
			else
				return;
		}
		if (_activeGamepad != -1 && glfwGetGamepadState(_activeGamepad, &_gamepadState))
		{
			// 更新所有已注册的手柄按钮状态, 不更新过去状态
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





#include "Event_InputPackages.h"

	void InputSystem::handleKey(int key, int scancode, int action, int mods)
	{
		UniversalInput input = KeyboardInput{ key };
		auto& state = getInputState(input);

		if (action == GLFW_PRESS)
		{
			state.pressCount++;
			state.lastPressTime = (float)glfwGetTime();

			EventRegistry::publish(Event_KeyDown::staticGetType(),
				[this, key, mods](IEvent* event) {
					auto e = static_cast<Event_KeyDown*>(event);
					e->key = key;
					e->modifiers = mods;
				});
		}
		else if (action == GLFW_RELEASE) {
			// 发布鼠标释放事件
			EventRegistry::publish(Event_KeyUp::staticGetType(),
				[this, key, mods](IEvent* event) {
					auto e = static_cast<Event_KeyUp*>(event);
					e->key = key;
					e->modifiers = mods;
				});
		}
	}

	void InputSystem::handleMouseButton(int button, int action, int mods)
	{
		UniversalInput input = MouseButtonInput{ button };
		auto& state = getInputState(input);

		if (action == GLFW_PRESS)
		{
			state.pressCount++;  // 记录按下次数（用于双击检测）
			state.lastPressTime = (float)glfwGetTime();
			state.pressPosition = _currentMousePos; // 记录按下时的鼠标位置

			// 发布鼠标按下事件
			EventRegistry::publish(Event_MouseButtonDown::staticGetType(),
				[this, button, mods](IEvent* event) {
					auto e = static_cast<Event_MouseButtonDown*>(event);
					e->button = button;
					e->mousePos = _currentMousePos;
					e->modifiers = mods;
				});
		}
		else if (action == GLFW_RELEASE) {
			// 发布鼠标释放事件
			EventRegistry::publish(Event_MouseButtonUp::staticGetType(),
				[this, button, mods](IEvent* event) {
					auto e = static_cast<Event_MouseButtonUp*>(event);
					e->button = button;
					e->mousePos = _currentMousePos;
					e->modifiers = mods;
				});
		}
	}

	void InputSystem::handleMousePosition(double x, double y)
	{
		_currentMousePos = glm::vec2((float)x, (float)y);

		EventRegistry::publish(Event_MouseMove::staticGetType(),
			[this](IEvent* event) {
				auto e = static_cast<Event_MouseMove*>(event);
				e->mousePos = _currentMousePos;
			});
	}

	void InputSystem::handleScroll(double xoffset, double yoffset)
	{
		_scrollDelta = glm::vec2(xoffset, yoffset);

		// 更新轴状态
		UniversalInput xInput{ MouseAxisInput{MouseAxis::ScrollX} };
		UniversalInput yInput{ MouseAxisInput{MouseAxis::ScrollY} };

		getInputState(xInput).value = xoffset;
		getInputState(yInput).value = yoffset;

		EventRegistry::publish(Event_Scroll::staticGetType(),
			[this](IEvent* event) {
				auto e = static_cast<Event_Scroll*>(event);
				e->scrollDelta = _scrollDelta;
				e->mousePos = _currentMousePos;
			});
	}

	void InputSystem::keyCallbackWrapper(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		auto inputSystem = GET_CAST_MODULE(InputSystem, "InputSystem");
		if (inputSystem) inputSystem->handleKey(key, scancode, action, mods);
	}

	void InputSystem::mouseButtonCallbackWrapper(GLFWwindow* window, int button, int action, int mods)
	{
		auto inputSystem = GET_CAST_MODULE(InputSystem, "InputSystem");
		if (inputSystem) inputSystem->handleMouseButton(button, action, mods);
	}

	void InputSystem::cursorPosCallbackWrapper(GLFWwindow* window, double xpos, double ypos)
	{
		auto inputSystem = GET_CAST_MODULE(InputSystem, "InputSystem");
		if (inputSystem) inputSystem->handleMousePosition(xpos, ypos);
	}

	void InputSystem::scrollCallbackWrapper(GLFWwindow* window, double xoffset, double yoffset)
	{
		auto inputSystem = GET_CAST_MODULE(InputSystem, "InputSystem");
		if (inputSystem) inputSystem->handleScroll(xoffset, yoffset);
	}
}