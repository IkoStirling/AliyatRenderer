#pragma once
#include "IAYCamera.h"
#include "glm/gtc/matrix_transform.hpp"

namespace ayt::engine::render
{
	class AY2DCamera : public IAYCamera
	{
	public:
		IAYCamera::Type getType() const override
		{
			return IAYCamera::Type::ORTHOGRAPHIC_2D;
		}

		void update(float delta_time);

		math::Matrix4 getViewMatrix() const override;

		math::Matrix4 getProjectionMatrix() const override;

		void setViewBox(float near, float far);

		// left, right, bottom, top
		void setDeadzone(const math::Vector4& zone);

		void setTargetPosition(const math::Vector2& targetPos);

		void setCurrentPosition(const math::Vector2& currentPos);

		void showDeadzone(bool switcher);
	private:
		math::Vector4 _deadzone{ 0.3f, 0.7f, 0.3f, 0.7f }; // 屏幕比例死区(摄像机不会跟随的四边形区域）
		math::Vector4 _mapBounds{ -500.f, 500.f, -500.f, 500.f };
		float _moveSpeed = 5.f;
		float _near = -500.f;
		float _far = 500.f;

		//debug
		int _ddeadzone = 0;
	};
}