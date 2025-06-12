#pragma once
#include "core/core.h"

#define getWorld() static_cast<Object*>(World::getInstance())

class World : public Object
{
public:
	static World* getInstance()
	{
		if (!worldInstance)
		{
			worldInstance = new World();
		}
		return worldInstance;
	}
private:
	World() :Object(nullptr) {}
	~World() { delete worldInstance; }

	World(const World&) = delete;
	World(World&&) = delete;

	static World* worldInstance;


};

World* World::worldInstance = World::getInstance();