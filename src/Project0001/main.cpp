#include "AYEngineCore.h"
#include "AYSceneManager.h"
#include "include/Scene_Level000.h"

void prepare()
{
	auto sm = GET_CAST_MODULE(AYSceneManager, "SceneManager");
	sm->addScene<Scene_Level000>("Level0");
}

int main()
{
	AYEngineCore::getInstance().init();
	prepare();
	AYEngineCore::getInstance().start();
	return 0;
}