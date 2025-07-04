#pragma once
#include "AYEntrant.h"
#include "Component/AYSpriteRenderComponent.h"
#include "Component/AYCameraComponent.h"
#include "AYPath.h"
#include "BaseRendering/Camera/AY3DCamera.h"

class Orc : public AYEntrant
{
public:
    Orc(const std::string& name = "Orc") :
        AYEntrant(name)
    {
		_orcSprite = addComponent<AYSpriteRenderComponent>("_orcSprite");
		_camera.push_back(addComponent<AYCameraComponent>("_orc2DCamera"));
		_camera.push_back(addComponent<AYCameraComponent>("_orc3DCamera"));
		_camera[0]->setupCamera(IAYCamera::Type::ORTHOGRAPHIC_2D);
		_camera[1]->setupCamera(IAYCamera::Type::PERSPECTIVE_3D);
		_camera[0]->activate();

		_orcSprite->setup_sprite(
			_name,
			AYPath::Engine::getPresetTexturePath() + "Orc.png",
			glm::vec2(100, 100),
			glm::vec2(800, 600),
			{
				{"idle01", 0, 6, true},
				{"walk01", 8, 8, true},
				{"atk01", 16, 6, false},
				{"atk02", 24, 6, false},
				{"eff01", 32, 4, false},
				{"dead01",40, 4, false}
			}
		);
		_orcSprite->playAnimation("idle01");
    }

    virtual void beginPlay()override
    {
		auto renderManager = GET_CAST_MODULE(AYRendererManager, "Renderer");
		auto device = renderManager->getRenderDevice();
		glfwSetInputMode(device->getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

	void processMouseMovement(float xpos, float ypos) 
	{
		auto renderManager = GET_CAST_MODULE(AYRendererManager, "Renderer");
		auto device = renderManager->getRenderDevice();


		static int lastX = 0, lastY = 0;
		float delterX = xpos - lastX;
		float delterY = ypos - lastY;
		lastX = xpos;
		lastY = ypos;

		float xoffset = delterX * _mouseSensitivity;
		float yoffset = delterY * _mouseSensitivity;

		
		auto cameraSystem = renderManager->getCameraSystem();
		auto cam3D = dynamic_cast<AY3DCamera*>(cameraSystem->getActiveCamera());
		if (cam3D)
		{
			cam3D->rotate(xoffset, yoffset);
		}
	}

	void setMouseSensitivity(float sensitivity) {
		_mouseSensitivity = sensitivity;
	}

    virtual void update(float delta_time)override
    {
        AYEntrant::update(delta_time);
		auto inputSystem = GET_CAST_MODULE(Mod_InputSystem, "InputSystem");

		processMouseMovement(inputSystem->getMousePosition().x, inputSystem->getMousePosition().y);

		if (_orcSprite->isCurrentAnimationDone())
		{
			static int switcher = 0;
			glm::vec3 movement(0.0f);
			float moveSpeed = 200.f * (inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_LEFT_SHIFT }) ? 5.f : 1.f);
			movement.x = inputSystem->getAxisValue(GamepadAxisInput{ GamepadAxis::LeftX, moveSpeed });
			movement.y = inputSystem->getAxisValue(GamepadAxisInput{ GamepadAxis::LeftY, moveSpeed });
			if (inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_A })) movement.x = -moveSpeed;
			if (inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_D })) movement.x += moveSpeed;
			if (inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_W })) movement.z = -moveSpeed;
			if (inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_S })) movement.z += moveSpeed;
			if (inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_SPACE })) movement.y = moveSpeed;
			if (inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_LEFT_ALT })) movement.y -= moveSpeed;

			if (inputSystem->getUniversalInputState(MouseButtonInput{ GLFW_MOUSE_BUTTON_LEFT }) ||
				inputSystem->isActionActive("default.GamePad_X"))
			{
				_orcSprite->playAnimation("atk01");
			}
			else if (inputSystem->getUniversalInputState(MouseButtonInput{ GLFW_MOUSE_BUTTON_RIGHT }))
			{
				_orcSprite->playAnimation("atk02");
			}
			else if (inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_F }))
			{
				_orcSprite->playAnimation("eff01");
			}
			else if (inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_K }))
			{
				_orcSprite->playAnimation("dead01");
			}
			else if (inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_C }))
			{
				std::cout << "C\n";
				_camera[switcher]->shake(100.f, 1.5f);
			}
			else if (inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_V }))
			{
				static int rad = 0;
				auto& trans = getTransform();
				setRotation(glm::vec3( 0.f,0.f,glm::radians((float)(rad++ % 160))));
			}
			else if (inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_Q }))
			{
				static int duration = 0;
				switcher = 1 - switcher;
				duration--;
				if (duration < 0)
				{
					_camera[switcher]->activate();
					duration = 20;
				}
			}
			else if (movement != glm::vec3(0.0f))
			{
				auto& trans = getTransform();
				setPosition(trans.position + glm::vec3(movement * delta_time));
				// 设置朝向
				if (movement.x != 0) {
					_orcSprite->setFlip(movement.x < 0, false);
				}
				_orcSprite->playAnimation("walk01");
			}
			else
			{
				_orcSprite->playAnimation("idle01");
			}
		}

    }
    virtual void endPlay() override
    {
    
    }


private:
    AYSpriteRenderComponent* _orcSprite;
	std::vector<AYCameraComponent*> _camera;

	float _mouseSensitivity = 0.1f; // 鼠标灵敏度
};