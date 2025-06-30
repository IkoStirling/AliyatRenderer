#pragma once
#include "AYGameObject.h"
#include "STTransform.h"


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