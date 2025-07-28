#pragma once
enum class AYEventLayer : size_t
{
	GAME_LOGIC = 0,
	PHYSICS = 1,
	RENDER = 2,
	NETWORK = 3,
	RESOURCE = 4,

	END,
};

static const int LAYER_NUMS = static_cast<size_t>(AYEventLayer::END);