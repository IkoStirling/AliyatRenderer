#pragma once
namespace ayt::engine::event
{
	enum class EventLayer : size_t
	{
		GAME_LOGIC = 0,
		PHYSICS = 1,
		RENDER = 2,
		NETWORK = 3,
		RESOURCE = 4,
		UI = 5,
		INPUT = 6,
		END,
	};

	static const int LAYER_NUMS = static_cast<size_t>(EventLayer::END);
}