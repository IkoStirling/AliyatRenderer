# AliyatRenderer
---
### Project Architecture
	--Core
		--EngineCore
		--ModuleManager
		--MemoryPool
		--EventSystem
		--ResourceManager
		--Renderer
		--Network
		--Physics (Not yet supported)
		--InputSystem
		--SceneManager (Not yet supported)
		--Logger (Not yet supported, should use spdlog?)
		//some system header files
### Project Constraints
1. Regarding the uniformity of project naming
 
	Class name prefix

		AY Core Components
		UT tool component
		I Interface/Abstract Class
		ST structure

    Code Style

		Class name (without prefix) with large camel hump
		Member functions and variables use small humps
		Private variables use underscore prefix
### Project Progress
1. This Orc can move, turn, and attack, and it receives the left joystick axis value input from Gmaepad, as well as the "X" as the attack key. Of course, it is currently only an animation

	![pic](assets/core/.projectProgress/orc_controlled_by_gamepad.png)

