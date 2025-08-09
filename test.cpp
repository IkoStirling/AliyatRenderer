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

	auto se = GET_CAST_MODULE(AYSoundEngine, "SoundEngine");
	auto& rm = AYResourceManager::getInstance();
	auto es = GET_CAST_MODULE(AYEventSystem, "EventSystem");
	//se->play2D(AYPath::Engine::getPresetAudioPath() + "bite-small.wav");
	//se->play3D("@audios/ambient/shade6.wav",glm::vec3(20),true,0.5f);
	//se->play3D("@audios/bite-small.wav",glm::vec3(20),false,true, 0.5f);
	//rm.loadAsync<AYVideo>("@videos/test_video.mp4",
	//	[](std::shared_ptr<AYVideo> video) {
	//		video->updateFrame();
	//		std::cout << video->getTextureID() << std::endl;
	//	});
	//se->play3D(AYPath::Engine::getPresetAudioPath() + "shade6.wav",glm::vec3(20),true);
	//se->play3D(AYPath::Engine::getPresetAudioPath() + "shade6.wav",glm::vec3(20),true);

	AYEngineCore::getInstance().start();
	return 0;
}