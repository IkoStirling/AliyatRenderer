#pragma once
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
	void setDeadzone(const glm::vec4& zone) { _deadzone = zone; } // left, right, bottom, top
	void setTargetPosition(const glm::vec2& targetPos) { _targetPosition = glm::vec3(targetPos, 1.f); }
	void setCurrentPosition(const glm::vec2& currentPos) { _transform.position = glm::vec3(currentPos,1.f); }
	glm::vec2 getScreenCenter() const { return  glm::vec2(_viewport.z, _viewport.w) * 0.5f; }
private:
	glm::vec4 _deadzone{ 0.3f, 0.7f, 0.3f, 0.7f }; // ��Ļ��������(��������������ı�������
	glm::vec4 _mapBounds{ -5000.f, 5000.f, -5000.f, 5000.f };

	float _moveSpeed = 5.f;

	float _near = -5000.f;
	float _far = 5000.f;
};