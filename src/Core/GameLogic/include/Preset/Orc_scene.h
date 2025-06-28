#pragma once
#include "IAYScene.h"
#include "Orc.h"

class Orc_scene : public IAYScene
{
public:
    virtual void load()
    {
        auto orc = addObject<Orc>("Orc");
    }
    virtual void asyncLoad()
    {

    }
    virtual void unload()
    {

    }

};