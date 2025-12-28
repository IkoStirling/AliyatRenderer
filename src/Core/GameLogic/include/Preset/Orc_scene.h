#pragma once
#include "IAYScene.h"
#include "Orc.h"
#include "Ground.h"

class Orc_scene : public IScene
{
public:
    virtual void load()
    {
        auto orc = addObject<Orc>("Orc");
        auto ground = addObject<Ground>("Ground");
    }
    virtual void asyncLoad()
    {

    }
    virtual void unload()
    {

    }

};