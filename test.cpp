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

	auto soundEngine = GET_CAST_MODULE(AYSoundEngine, "SoundEngine");
	//soundEngine->playSound2D("@videos/bad_apple.mp4", false, false,1,false);
	//soundEngine->playVideo("@videos/bad_apple.mp4");

	//getchar();
	AYEngineCore::getInstance().start();
	return 0;
}