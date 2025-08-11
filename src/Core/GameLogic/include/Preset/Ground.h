#pragma once
#include "AYEntrant.h"
#include "Component/AYSpriteRenderComponent.h"

class Ground : public AYEntrant
{
public:
	Ground(const std::string& name = "Ground") :
		AYEntrant(name)
	{
		auto physicsSystem = GET_CAST_MODULE(AYPhysicsSystem, "PhysicsSystem");
		auto renderManager = GET_CAST_MODULE(AYRendererManager, "Renderer");
		_renderer = renderManager->getRenderer();
		_device = renderManager->getRenderDevice();
		auto ecsEngine = GET_CAST_MODULE(AYECSEngine, "ECSEngine");
		auto entity2 = ecsEngine->createEntity();
		ecsEngine->addComponent<STTransform>(entity2);
		auto ground = physicsSystem->getPhysicsWorld(WorldType::AY2D)
			->createBody(
				entity2,
				glm::vec2(0, -1),
				0.f,
				IAYPhysicsBody::BodyType::Static);
		ground->addCollider(new Box2DBoxCollider(glm::vec2(5000, 1)));
	

	}

	virtual void beginPlay()override
	{
		AYEntrant::beginPlay();
		auto& reousceManager = AYResourceManager::getInstance();
		auto tex = reousceManager.load<AYTexture>("@textures/checkerboard.png");
		_texID = _device->createTexture2D(tex->getPixelData(), tex->getWidth(), tex->getHeight(), tex->getChannels());
	}

	virtual void update(float delta_time)override
	{
		if (_texID>0)
		{
			_renderer->getSpriteRenderer()->drawSprite(
				_texID,
				getTransform(),
				glm::vec2(1920, 1080),  // 大小
				glm::vec4(1.0f, 1.f, 1.f, 0.9f),// 颜色
				false,
				false,
				glm::vec2(0.5f, 0.5f)       // 原点(旋转中心)
			);
		}
		// 5. 调试输出（临时）
		static float time = 0;
		time += delta_time;
		if (time > 1.f)
		{
			time -= 1.f;
			auto pos = _physics->getPhysicsBody()->getPosition();
			std::cout << std::fixed << std::setprecision(4)
				<< "[Ground]"
				<< ")  \tPosition(" << pos.x << ", " << pos.y << ")\n";
		}
	}

	virtual void endPlay() override
	{

	}
private:
	AYRenderDevice* _device;
	AYRenderer* _renderer;
	GLuint _texID;
};