#include "IAYScene.h"

class Scene_Level000 : public IAYScene
{
public:
    virtual void load()override;       // 同步加载
    virtual void unload()override;
    virtual void update(float dt)override;
    virtual void render()override;

    // 异步加载接口
    virtual bool isLoaded() const override;
    virtual float getLoadProgress() const override;
private:

};