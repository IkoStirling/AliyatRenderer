#pragma once
#include "IAYRenderComponent.h"

class AYSpriteRenderComponent : public IAYRenderComponent
{
public:
	virtual void init() override {};
	virtual void update(float delta_time) override {};
	virtual void render(const AYRenderContext& context) override
	{
		std::cout << "AYSpriteRenderComponent\n";
	}
};