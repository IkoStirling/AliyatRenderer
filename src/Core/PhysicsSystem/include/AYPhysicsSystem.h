#pragma once
#include "Mod_PhysicsSystem.h"
#include "ECPhysicsDependence.h"
#include "BasePhy/IAYPhysicsWorld.h"
#include "BasePhy/STRaycastResult.h"
#include "2DPhy/Box2DPhysicsWorld.h"

namespace ayt::engine::physics
{
	namespace CategoryBits {
		enum Type : uint16_t {
			None = 0x0000,		// 无类别（通常不用作物体的类别）
			Player = 0x0001,		// 玩家角色
			Enemy = 0x0002,		// 敌人
			Ground = 0x0004,		// 地面 / 平台
			Wall = 0x0008,		// 墙壁 / 障碍物
			Bullet = 0x0010,		// 子弹
			PowerUp = 0x0020,		// 道具 / 加血 / 加分等
			Collectible = 0x0040,		// 可收集物品
			Projectile = 0x0080,		// 投射物（比如火球、箭矢）
			Sensor = 0x0100,		// 传感器（比如触发区域）
			NPC = 0x0200,		// NPC
			Vehicle = 0x0400,		// 车辆 / 可移动平台

			BlockAll = 0xFFFF,
		};
	}

	class PhysicsSystem : public Mod_PhysicsSystem
	{
	public:
		PhysicsSystem() = default;
		~PhysicsSystem() = default;

		virtual void init() override;
		virtual void update(float delta_time) override;
		virtual void shutdown() override {}

		void addToWorld(EntityID entity, WorldType type);

		void createWorld(WorldType type);
		void destroyWorld(WorldType type);
		IPhysicsWorld* getPhysicsWorld(WorldType type);

		void setSimulationPaused(bool paused);
		void setFixedTimeStep(float timeStep);

		void setDebugDrawEnabled(bool enabled);
		void setDebugDrawFlags(uint32_t flags);

	private:
		void _syncPhysicsToLogic();
		void _syncLogicToPhysics();
		struct PhysicsWorld {
			std::unique_ptr<IPhysicsWorld> impl;
			std::unordered_set<EntityID> entities;
		};
		std::unordered_map<WorldType, PhysicsWorld> _worlds;

		bool _paused = false;

		bool _isFixedPhysicsSimulater = true;
		float _fixedTimeStep = 1.0f / 60.0f;
		int _velocityIterations = 8;
		int _positionIterations = 3;

		bool _debugDrawEnabled = false;
		uint32_t _debugDrawFlags = 0;
	};

	REGISTER_MODULE_CLASS("PhysicsSystem", PhysicsSystem)
}