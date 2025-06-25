#include "IAYScene.h"

class Scene_Level000 : public IAYScene
{
public:
    virtual void load()override;       // ͬ������
    virtual void unload()override;
    virtual void update(float dt)override;
    virtual void render()override;

    // �첽���ؽӿ�
    virtual bool isLoaded() const override;
    virtual float getLoadProgress() const override;
private:

};