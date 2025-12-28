#include "AYPhysicsSystem.h"
#include "AYRendererManager.h"
namespace ayt::engine::physics
{
	using namespace ::ayt::engine::render;
	using namespace ::ayt::engine::ecs;

	void AYPhysicsSystem::init()
	{
		createWorld(WorldType::AY2D);
		createWorld(WorldType::AY3D);
		setDebugDrawEnabled(true);
	}

	void AYPhysicsSystem::update(float delta_time)
	{
		if (_paused) return;

		_syncLogicToPhysics();


		for (auto& [type, world] : _worlds)
		{
			world.impl->step(_isFixedPhysicsSimulater ? _fixedTimeStep : delta_time, _velocityIterations, _positionIterations);
		}

		_syncPhysicsToLogic();
	}

	void AYPhysicsSystem::addToWorld(EntityID entity, WorldType type)
	{
		auto& world = _worlds[type];
		if (world.impl->createBody(entity)) {
			world.entities.insert(entity);
		}
	}

	void AYPhysicsSystem::createWorld(WorldType type)
	{
		auto it = _worlds.find(type);
		if (it != _worlds.end())
			return;

		switch (type)
		{
		case WorldType::AY2D:
			_worlds.emplace(WorldType::AY2D, PhysicsWorld{ std::make_unique<Box2DPhysicsWorld>() });
			break;
		case WorldType::AY3D:
			break;
		default:
			break;
		}
	}

	void AYPhysicsSystem::destroyWorld(WorldType type)
	{
		_worlds.erase(type);
	}

	IAYPhysicsWorld* AYPhysicsSystem::getPhysicsWorld(WorldType type)
	{
		auto it = _worlds.find(type);
		if (it != _worlds.end())
		{
			return it->second.impl.get();
		}
		return nullptr;
	}

	void AYPhysicsSystem::setSimulationPaused(bool paused)
	{
		_paused = paused;
	}

	void AYPhysicsSystem::setFixedTimeStep(float timeStep)
	{
		_fixedTimeStep = timeStep;
	}

	void AYPhysicsSystem::setDebugDrawEnabled(bool enabled)
	{
		auto rendererManager = GET_CAST_MODULE(AYRendererManager, "Renderer");
		if (enabled && !_debugDrawFlags)
		{
			_debugDrawFlags = rendererManager->addDebugDraw(false, [this](AYRenderer* renderer, AYRenderDevice* device) {
				for (auto& [type, world] : _worlds)
				{
					for (auto body : world.impl->getAllBodies())
					{
						if (auto bbody = dynamic_cast<Box2DPhysicsBody*>(body))
						{
							auto aabbVec4 = bbody->getAABB();
							// 解包
							float minX = aabbVec4.x;
							float minY = aabbVec4.y;
							float maxX = aabbVec4.z;
							float maxY = aabbVec4.w;

							// 计算中心点
							float centerX = (minX + maxX) * 0.5f;
							float centerY = (minY + maxY) * 0.5f;

							// 计算宽高
							float width = maxX - minX;
							float height = maxY - minY;

							// 转换为 math::Vector2
							math::Vector3 center(centerX, centerY, 0);
							math::Vector2 size(width, height);
							renderer->getCoreRenderer()
								->drawRect2D(
									{ center },
									size,
									0,
									true,
									AYCoreRenderer::Space::World);
						}
					}
				}
				});
		}
		else if (!enabled && _debugDrawFlags)
		{
			rendererManager->removeDebugDraw(_debugDrawFlags);
		}
	}

	void AYPhysicsSystem::setDebugDrawFlags(uint32_t flags)
	{
		_debugDrawFlags = flags;
	}

	void AYPhysicsSystem::_syncLogicToPhysics()
	{
		auto view = GET_CAST_MODULE(ECS, "ECSEngine")->getView<math::Transform, STPhysicsComponent>();

		for (auto [entity, transform, physics] : view)
		{
			_worlds[physics.worldType].impl->setTransform(entity, transform);
		}
	}

	void AYPhysicsSystem::_syncPhysicsToLogic()
	{
		auto view = GET_CAST_MODULE(ECS, "ECSEngine")->getView<math::Transform, STPhysicsComponent>();

		for (auto [entity, transform, physics] : view)
		{
			auto trans = _worlds[physics.worldType].impl->getTransform(entity);
			physics.lastTransform = transform;
			transform = trans;
		}

		// 测试用
		auto view2 = GET_CAST_MODULE(ECS, "ECSEngine")->getView<math::Transform>();
		for (auto [entity, transform] : view2)
		{
			auto trans = _worlds[WorldType::AY2D].impl->getTransform(entity);
			transform = trans;
		}
	}
}