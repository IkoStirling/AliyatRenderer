#pragma once
#include "AYEntrant.h"
#include "Component/AYSpriteRenderComponent.h"
#include "Component/AYCameraComponent.h"
#include "AYPath.h"

class Orc : public AYEntrant
{
public:
    Orc(const std::string& name = "Orc") :
        AYEntrant(name)
    {
		_orcSprite = addComponent<AYSpriteRenderComponent>("_orcSprite");
		_camera = addComponent<AYCameraComponent>("_orcCamera");
		_camera->setupCamera(IAYCamera::Type::ORTHOGRAPHIC_2D);
		_camera->active();

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

    }

    virtual void update(float delta_time)override
    {
        AYEntrant::update(delta_time);

		if (_orcSprite->isCurrentAnimationDone())
		{
			auto inputSystem = GET_CAST_MODULE(Mod_InputSystem, "InputSystem");

			glm::vec2 movement(0.0f);
			float moveSpeed = 200.f * (inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_LEFT_SHIFT }) ? 5.f : 1.f);
			movement.x = inputSystem->getAxisValue(GamepadAxisInput{ GamepadAxis::LeftX, moveSpeed });
			movement.y = inputSystem->getAxisValue(GamepadAxisInput{ GamepadAxis::LeftY, moveSpeed });
			if (inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_A })) movement.x = -moveSpeed;
			if (inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_D })) movement.x += moveSpeed;
			if (inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_W })) movement.y = -moveSpeed;
			if (inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_S })) movement.y += moveSpeed;

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
			else if (inputSystem->getUniversalInputState(KeyboardInput{ GLFW_KEY_V }))
			{
				static int rad = 0;
				auto& trans = getTransform();
				setRotation(glm::vec3( 0.f,0.f,glm::radians((float)(rad++ % 160))));
			}
			else if (movement != glm::vec2(0.0f))
			{
				auto& trans = getTransform();
				setPosition(trans.position + glm::vec3(movement * delta_time, 0.0f));
				// ÉèÖÃ³¯Ïò
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
	AYCameraComponent* _camera;
};