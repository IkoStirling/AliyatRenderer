#pragma once
#include "AYEntrant.h"
#include "Component/AYSpriteRenderComponent.h"

class Ground : public AYEntrant
{
public:
	Ground(const std::string& name = "Ground") :
		AYEntrant(name)
	{
		_sprite = addComponent<AYSpriteRenderComponent>("Ground");

		auto physicsSystem = GET_CAST_MODULE(AYPhysicsSystem, "PhysicsSystem");
		auto renderManager = GET_CAST_MODULE(AYRendererManager, "Renderer");

		auto ecsEngine = GET_CAST_MODULE(AYECSEngine, "ECSEngine");
		auto entity2 = ecsEngine->createEntity();
		ecsEngine->addComponent<STTransform>(entity2);
		setPosition(glm::vec2(0, 0));
		auto ground = physicsSystem->getPhysicsWorld(WorldType::AY2D)
			->createBody(
				entity2,
				getPosition(),
				0.f,
				IAYPhysicsBody::BodyType::Static);
		auto collider = new Box2DBoxCollider(glm::vec2(500, 1));
		collider->setOffset(glm::vec2(0, -0.5f)); 
		ground->addCollider(collider);

	}

	virtual void beginPlay()override
	{
		AYEntrant::beginPlay();
		auto& reousceManager = AYResourceManager::getInstance();
		auto tex = reousceManager.load<AYTexture>("@textures/checkerboard.png");
		auto renderManager = GET_CAST_MODULE(AYRendererManager, "Renderer");
		auto device = renderManager->getRenderDevice();
		_texID = device->createTexture2D(tex->getPixelData(), tex->getWidth(), tex->getHeight(), tex->getChannels());
		_sprite->setup_picture(
			_texID,
			glm::vec2(0),
			glm::vec2(500,1),
			glm::vec3(500,1,0),
			glm::vec4(1),
			glm::vec3(0)
		);
	}

	virtual void update(float delta_time)override
	{
		// 5. 调试输出（临时）
		static float time = 0;
		time += delta_time;
		if (time > 1.f)
		{
			time -= 1.f;
			auto pos = _physics->getPhysicsBody()->getPosition();
			//std::cout << std::fixed << std::setprecision(4)
			//	<< "[Ground]"
			//	<< ")  \tPosition(" << pos.x << ", " << pos.y << ")\n";
		}
	}

	virtual void endPlay() override
	{

	}
private:
	GLuint _texID;
	AYSpriteRenderComponent* _sprite;
};