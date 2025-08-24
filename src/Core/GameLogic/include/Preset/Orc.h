#pragma once
#include "AYEntrant.h"
#include "Component/AYSpriteRenderComponent.h"
#include "Component/AYCameraComponent.h"
#include "AYPath.h"
#include "BaseRendering/Camera/AY3DCamera.h"
#include "Component/AYPlayerController.h"
#include "2DPhy/Collision/Box2D/Box2DBoxCollider.h"
#include "AYSoundEngine.h"

class Orc : public AYEntrant
{
public:
    Orc(const std::string& name = "Orc") :
        AYEntrant(name)
    {
		_orcSprite = addComponent<AYSpriteRenderComponent>("_orcSprite");
		_physics->addCollider(std::make_shared<Box2DBoxCollider>());
		_physics->setBodyType(IAYPhysicsBody::BodyType::Dynamic);
		_controller = addComponent<AYPlayerController>("_controller");
		_controller->setMoveSpeed(5.0f);
		_controller->setJumpForce(10.0f);
		_camera.push_back(addComponent<AYCameraComponent>("_orc2DCamera"));
		_camera.push_back(addComponent<AYCameraComponent>("_orc3DCamera"));
		_camera[0]->setupCamera(IAYCamera::Type::ORTHOGRAPHIC_2D);
		_camera[1]->setupCamera(IAYCamera::Type::PERSPECTIVE_3D);
		_camera[0]->activate();

		_orcSprite->setup_sprite(
			_name,
			AYPath::Engine::getPresetTexturePath() + "Orc.png",
			glm::vec2(16,9),
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
		_orcSprite->setSize(glm::vec3(1.5f));
		//	注意渲染精灵朝向问题，
		// _transform.rotation.y = 180.f;
    }

    virtual void beginPlay()override
    {
		AYEntrant::beginPlay();
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
			movement.x = inputSystem->getAxisValue(GamepadAxisInput{ GamepadAxis::LeftX});
			movement.y = inputSystem->getAxisValue(GamepadAxisInput{ GamepadAxis::LeftY});
			if (inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_A })) movement.x = -1;
			if (inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_D })) movement.x += 1;
			if (inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_W })) movement.z = -1;
			if (inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_S })) movement.z += 1;
			if (inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_SPACE })) movement.y = 1;
			if (inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_LEFT_ALT })) movement.y -= 1;
			float moveSpeed = 1.f * (inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_LEFT_SHIFT }) ? 5.f : 1.f);

			if (glm::length(movement) > 0.0f) {
				movement = glm::normalize(movement);
			}


			
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
			else if (inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_KP_0 }))
			{
				static bool flag0 = true;
				if (flag0)
				{
					//GET_CAST_MODULE(AYSoundEngine, "SoundEngine")->play2D("@audios/ambient/amb_dark_01.wav", true, true);
					GET_CAST_MODULE(AYSoundEngine, "SoundEngine")->play2D("@audios/ambient/Evening_wanders.mp3", true, true, 0.5f);
					flag0 = false;
				}
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
				duration--;
				if (duration < 0)
				{
					switcher = 1 - switcher;
					_camera[switcher]->activate();
					duration = 20;
				}
			}
			else
			{
				static float baseZoom = 1.f;
				float deltaY = inputSystem->getScrollDelta(MouseAxisInput{ MouseAxis::ScrollY });
				if (deltaY > 0.99f || deltaY < -0.99f)
				{
					deltaY = glm::clamp(deltaY, -1.0f, 1.0f);
					baseZoom = glm::clamp(baseZoom + deltaY * 0.1f, 0.5f, 10.0f);
					_camera[switcher]->getCamera()->setZoom(baseZoom);
				}

				auto* camera = _camera[switcher]->getCamera();
				auto* cam3D = dynamic_cast<AY3DCamera*>(camera);
				if (cam3D)
				{
					_physics->setPhysicsMode(true);
					_orcSprite->setVisible(false);
					glm::vec3 cameraFront = cam3D->getFront();
					glm::vec3 cameraRight = cam3D->getRight();

					// 计算实际移动方向
					glm::vec3 moveDirection = (cameraFront * -movement.z) +
						(cameraRight * movement.x) +
						(glm::vec3(0.0f, 1.0f, 0.0f) * movement.y);

					if (glm::length(moveDirection) > 0.0f) {
						moveDirection = glm::normalize(moveDirection);
					}

					// 应用移动
					auto& trans = getTransform();
					glm::vec3 zmove(moveDirection * moveSpeed * delta_time);
					setPosition(trans.position + zmove);

					// 设置精灵朝向
					if (movement.x != 0) {
						_orcSprite->setFlip(movement.x > 0, false);
					}
					_orcSprite->playAnimation("walk01");
				}
				else
				{
					_physics->setPhysicsMode(false);
					_orcSprite->setVisible(true);

					// 2D相机的原始移动逻辑
					if (movement != glm::vec3(0.0f)) {
						auto& trans = getTransform();

						if (movement.x != 0) {
							_orcSprite->setFlip(movement.x < 0, false);
						}
						_orcSprite->playAnimation("walk01");
					}
					else {
						_orcSprite->playAnimation("idle01");
					}
				}
			}

		}

    }
    virtual void endPlay() override
    {
    
    }


private:
    AYSpriteRenderComponent* _orcSprite;
	AYPlayerController* _controller;
	std::vector<AYCameraComponent*> _camera;

	float _mouseSensitivity = 0.1f; // 鼠标灵敏度
};