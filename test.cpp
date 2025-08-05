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
	es->update(0);
	rm.loadAsync<AYTexture>(AYPath::Engine::getPresetTexturePath() + "Arrow.png",
		[](std::shared_ptr<AYTexture> texture) {
			std::cout << "Texture loaded: " << texture->getWidth() << "x" << texture->getHeight() << std::endl;
		});
	rm.loadAsync<AYTexture>(AYPath::Engine::getPresetTexturePath() + "Arrow.png",
		[](std::shared_ptr<AYTexture> texture) {
			std::cout << "Texture loaded: " << texture->getWidth() << "x" << texture->getHeight() << std::endl;
		});
	rm.loadAsync<AYTexture>(AYPath::Engine::getPresetTexturePath() + "Orc.png",
		[](std::shared_ptr<AYTexture> texture) {
			std::cout << "Texture loaded: " << texture->getWidth() << "x" << texture->getHeight() << std::endl;
		});
	se->play2D(AYPath::Engine::getPresetAudioPath() + "shade6.wav");
	se->play2D(AYPath::Engine::getPresetAudioPath() + "bite-small.wav");
	rm.loadAsync<AYAudio>(AYPath::Engine::getPresetAudioPath() + "shade6.wav",
		[](std::shared_ptr<AYAudio> audio) {
			std::cout << "audio loaded: " << audio->getChannels()<< std::endl;
		});
	//se->play3D(AYPath::Engine::getPresetAudioPath() + "shade6.wav",glm::vec3(20),true);
	//se->play3D(AYPath::Engine::getPresetAudioPath() + "shade6.wav",glm::vec3(20),true);
	//se->play3D(AYPath::Engine::getPresetAudioPath() + "shade6.wav",glm::vec3(20),true);

	AYEngineCore::getInstance().start();
	return 0;
}