#pragma once
#include "AYGameObject.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

struct STTransform
{
	glm::vec3 position = glm::vec3(0.f);
	glm::vec3 rotation = glm::vec3(0.f);
	float scale = 1.f;

    glm::mat4 GetTransformMatrix() const {
        glm::mat4 matrix(1.0f);
        matrix = glm::translate(matrix, position);
        matrix = glm::rotate(matrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        matrix = glm::rotate(matrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        matrix = glm::rotate(matrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        matrix = glm::scale(matrix, glm::vec3(scale));
        return matrix;
    }
};

class AYEntrant : public AYGameObject
{
public:
	AYEntrant(const std::string& name = "Entrant");
	~AYEntrant();


public: // transform
    const STTransform& getTransform() const { return _transform; }
    void setTransform(const STTransform& transform) { _transform = transform; }

    glm::vec3 getPosition() const { return _transform.position; }
    void setPosition(const glm::vec3& position) { _transform.position = position; }

    glm::vec3 getRotation() const { return _transform.rotation; }
    void setRotation(const glm::vec3& rotation) { _transform.rotation = rotation; }

    float getScale() const { return _transform.scale; }
    void setScale(float scale) { _transform.scale = scale; }

protected:
    STTransform _transform;
};