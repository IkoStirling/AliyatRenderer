﻿#pragma once
#include "IAYCamera.h"
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"


class AY2DCamera : public IAYCamera
{
public:
	IAYCamera::Type getType() const override
	{
		return IAYCamera::Type::ORTHOGRAPHIC_2D;
	}

	void update(float delta_time);

	glm::mat4 getViewMatrix() const override;

	glm::mat4 getProjectionMatrix() const override;

	void setViewBox(float near, float far);
	float getPixelPerMeter() const override
	{
		// 假设默认 1米 = 100像素（基础比例）
		const float BASE_PPM = 66.7f;
		// 动态缩放后的 PPM
		return BASE_PPM * _zoom;
	}
	void setDeadzone(const glm::vec4& zone) { _deadzone = zone; } // left, right, bottom, top
	void setTargetPosition(const glm::vec2& targetPos) { _targetPosition = glm::vec3(targetPos, 0.f) * getPixelPerMeter(); }
	void setCurrentPosition(const glm::vec2& currentPos) { _transform.position = glm::vec3(currentPos,0.f) * getPixelPerMeter(); }
	glm::vec2 getScreenCenter() const { return  glm::vec2(_viewport.z / _zoom, _viewport.w / _zoom) * 0.5f; }
private:
	glm::vec4 _deadzone{ 0.3f, 0.7f, 0.3f, 0.7f }; // 屏幕比例死区(摄像机不会跟随的四边形区域）
	glm::vec4 _mapBounds{ -5000.f, 5000.f, -5000.f, 5000.f };

	float _moveSpeed = 5.f;

	float _near = -5000.f;
	float _far = 5000.f;
};