#pragma once
#include <iostream>
#include "AYEngineCore.h"
#include "AYPhysicsSystem.h"
#include "2DPhy/Collision/Box2D/Box2DBoxCollider.h"
int main()
{
	auto physics = GET_CAST_MODULE(AYPhysicsSystem, "PhysicsSystem");
	physics->init();
	auto world = physics->getPhysicsWorld(AYPhysicsSystem::WorldType::AY2D);
	world->setGravity(glm::vec2(0, -9.8f));

	auto body = world->createBody(glm::vec2(), 0.f, IAYPhysicsBody::BodyType::Dynamic);
	auto bodyBox2D = static_cast<b2Body*>(static_cast<Box2DPhysicsBody*>(body)->getB2Body());
	auto boxC = std::make_shared<Box2DBoxCollider>();
	std::cout << "Initial position: (" << body->getPosition().x << ", " << body->getPosition().y << ")\n";
	body->addCollider(boxC.get());
	std::cout << "Mass: " << bodyBox2D->GetMass() << "\n";
	std::cout << "Friction: " << boxC->getFriction() << "\n";
	std::cout << "Density: " << boxC->getDensity() << "\n";
	std::cout << "Restitution: " << boxC->getRestitution() << "\n";
	body->applyForce(glm::vec2(100.0f, 0.0f));
	world->step(1.0f); 
	std::cout << "position: \t(" << body->getPosition().x << ", " << body->getPosition().y << ")\n";
	//body->applyImpulse(glm::vec2(0.0f, 50.0f));
	world->step(1.0f);  // 标准游戏帧率（60 FPS）
	std::cout << "position: \t(" << body->getPosition().x << ", " << body->getPosition().y << ")\n";
	AYEngineCore::getInstance().init();
	AYEngineCore::getInstance().start();
	return 0;
}