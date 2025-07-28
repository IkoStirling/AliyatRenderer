#pragma once
#include "STTransform.h"

class IAYPhysical
{
public: // transform
    const STTransform& getTransform() const { return _transform; }
    void setTransform(const STTransform& transform) { _transform = transform; }

    glm::vec3 getPosition() const { return _transform.position; }
    void setPosition(const glm::vec3& position) { _transform.position = position; }
    void setPosition(const glm::vec2& position) { _transform.position = glm::vec3(position, _transform.position.z); }

    glm::vec3 getRotation() const { return _transform.rotation; }
    void setRotation(const glm::vec3& rotation) { _transform.rotation = rotation; }
    void setRotation(float rotation) { _transform.rotation = glm::vec3(glm::vec2(_transform.rotation),rotation); }

    glm::vec3 getScale() const { return _transform.scale; }
    void setScale(const glm::vec3& scale) { _transform.scale = scale; }
    void setScale(const glm::vec2& scale) { _transform.scale = glm::vec3(scale, _transform.scale.z); }

protected:
    STTransform _transform;
};