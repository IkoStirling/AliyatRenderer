#include <iostream>
#include <Core/EngineCore/AYEngineCore.h>
#include "Core/EventSystem/AYEventSystem.h"
#include "Core/SystemEventType/Event_ResourceLoadAsync.h"
#include "Core/SystemResourceType/AYTexture.h"
#include "Core/ResourceManager/AYResourceManager.h"









int main()
{
	GetEngine().init();
	GetEngine().start();
	auto x = AYResourceManager::getInstance().loadAsync<AYTexture>("");
	GetEventSystem()->update();
	std::shared_ptr< Event_ResourceLoadAsync<AYTexture>> ptrS(new Event_ResourceLoadAsync<AYTexture>());
	auto u = ptrS->clone();
	std::cout << x.get()->test << std::endl;

	getchar();
	return 0;
}