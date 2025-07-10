#pragma once
#include "IAYRenderComponent.h"
#include "2DRendering/AYAnimationManager.h"
#include "AYEntrant.h"


class AYSpriteRenderComponent : public IAYRenderComponent
{
public:
	struct AnimationConfig {
		std::string name;
		int startFrame;
		int frameCount;
		bool loop;
	};

	virtual void beginPlay() override {};
	virtual void update(float delta_time) override 
	{
		if (_sprite) {
			_sprite->update(delta_time);
		}
	};
	virtual void endPlay() override {};
	virtual void render(const AYRenderContext& context) override
	{
		if (!_sprite)
			return;
		auto ent = dynamic_cast<AYEntrant*>(getOwner());
		if (!ent)
			return;
		auto& trans = ent->getTransform();

		if(_isVisible)
			_sprite->render(
				trans,
				_spriteSize,
				_color,
				_flipX,
				_flipY,
				_pivot
			);
	}

	void setVisible(bool visible)
	{
		_isVisible = visible;
	}

	void setup_sprite(
		const std::string& name,
		const std::string& texturePath,
		const glm::vec2& characterSize,
		const glm::vec2& atlasSize,
		const std::vector<AnimationConfig>& animations
	) {
		auto renderMgr = GET_CAST_MODULE(AYRendererManager, "Renderer");
		auto animMgr = renderMgr->get2DAnimationManager();

		// 转换动画配置格式
		std::vector<AnimeDataCreator> animeData;
		std::vector<bool> loops;
		for (const auto& anim : animations) {
			animeData.push_back({ anim.name, anim.startFrame, anim.frameCount });
			loops.push_back(anim.loop);
		}

		// 创建动画数据
		auto data = animMgr->makeAnimationData(characterSize, atlasSize, animeData);
		auto atlas = animMgr->loadAtlas(name, texturePath, characterSize, data, loops);

		// 创建精灵
		_sprite = std::unique_ptr<AYAnimatedSprite>(
			renderMgr->create2DSprite(atlas)
		);
		_spriteSize = glm::vec3(200); // 默认大小
	}

	void playAnimation(const std::string& name) {
		if (_sprite) {
			_sprite->playAnimation(name);
		}
	}

	bool isCurrentAnimationDone()
	{
		return _sprite->getController().isCurrentAnimationDone();
	}

	void setFlip(bool x, bool y)
	{
		_flipX = x;
		_flipY = y;
	}

	void setSize(const glm::vec3& size)
	{
		_spriteSize = size;
	}
	
public:
	std::unique_ptr< AYAnimatedSprite> _sprite;
	glm::vec3 _spriteSize = glm::vec3(1.f);
	glm::vec4 _color = glm::vec4(1.0f);
	bool _flipX = false;
	bool _flipY = false;
	glm::vec3 _pivot = glm::vec3(0.5f);
	bool _isVisible = true;
};