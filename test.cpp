#pragma once
#include <iostream>
#include "AYEngineCore.h"
#include "AYPhysicsSystem.h"
#include "2DPhy/Collision/Box2D/Box2DBoxCollider.h"
#include "AYECSEngine.h"
#include "AYSoundEngine.h"
#include "AYVideo.h"

struct T{};
struct S{};

int main()
{
	AYEngineCore::getInstance().init();
	AYEngineCore::getInstance().start();
	return 0;
}