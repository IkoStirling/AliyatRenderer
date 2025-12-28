#pragma once
#include "IAYRenderComponent.h"
#include "2DRendering/AYAnimationManager.h"
#include "AYEntrant.h"

namespace ayt::engine::game
{
	using namespace ::ayt::engine::render;

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
			if (!_sprite && _isAnimated)
				return;
			auto ent = dynamic_cast<AYEntrant*>(getOwner());
			if (!ent)
				return;
			auto& trans = ent->getTransform();
			if (_isVisible)
			{
				auto renderManager = GET_CAST_MODULE(AYRendererManager, "Renderer");
				auto spriteRenderer = renderManager->getRenderer()->getSpriteRenderer();
				auto cameraSystem = renderManager->getCameraSystem();
				float ppm = cameraSystem->getActiveCamera()->getPixelPerMeter();
				if (_isAnimated)
				{
					math::Transform spriteTrans = trans;
					spriteTrans.position.y += _characterSize.y * _spriteSize.y * 0.5f / ppm;
					_sprite->render(
						spriteTrans,
						_spriteSize,
						_color,
						_flipX,
						_flipY,
						_pivot
					);
				}
				else
				{
					//Transform spriteTrans = trans;
					//spriteTrans.position.x -= _spriteSize.x * 0.5f;
					//spriteRenderer->drawSprite(
					//	_texture,
					//	spriteTrans,
					//	_uvOffset,
					//	_uvSize,
					//	_spriteSize,
					//	_color,
					//	_flipX,
					//	_flipY,
					//	_pivot
					//);
				}
			}
		}

		void setVisible(bool visible)
		{
			_isVisible = visible;
		}

		void setAnimated(bool animated)
		{
			_isAnimated = animated;
		}

		void setup_picture(GLuint texture,
			const glm::vec2& uvOffset = glm::vec2(0.f),
			const glm::vec2& uvSize = glm::vec2(1.f),
			const glm::vec3& size = glm::vec3(1),
			const glm::vec4& color = glm::vec4(1),
			const glm::vec3& origin = glm::vec3(0)
		)
		{
			_isAnimated = false;
			_uvOffset = uvOffset;
			_uvSize = uvSize;
			_texture = texture;
			_spriteSize = size;
			_color = color;
			_pivot = origin;

		}

		void setup_sprite(
			const std::string& name,
			const std::string& texturePath,
			const glm::vec2& characterSize,
			const glm::vec2& spriteSize,
			const glm::vec2& atlasSize,
			const std::vector<AnimationConfig>& animations
		) {
			_isAnimated = true;
			_characterSize = characterSize;
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
			auto data = animMgr->makeAnimationData(spriteSize, atlasSize, animeData);
			auto atlas = animMgr->loadAtlas(name, texturePath, spriteSize, data, loops);

			// 创建精灵
			_sprite = std::unique_ptr<AYAnimatedSprite>(
				renderMgr->create2DSprite(atlas)
			);
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
		std::unique_ptr<AYAnimatedSprite> _sprite;
		glm::vec2 _uvOffset = glm::vec2(0);
		glm::vec2 _uvSize = glm::vec2(1);
		glm::vec3 _spriteSize = glm::vec3(1.f);
		glm::vec2 _characterSize = glm::vec2(1.f);
		glm::vec4 _color = glm::vec4(1.0f);
		bool _flipX = false;
		bool _flipY = false;
		glm::vec3 _pivot = glm::vec3(0.5f);
		bool _isVisible = true;
		bool _isAnimated = true;
		GLuint _texture;
	};
}