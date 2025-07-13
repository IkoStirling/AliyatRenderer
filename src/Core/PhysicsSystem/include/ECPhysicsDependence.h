#pragma once
#include <box2d/b2_math.h> 
#include <foundation/PxVec3.h>
#include <iostream>
#include <sstream>
#include <glm/glm.hpp>
#include <atomic>
#include <functional>

// GLM -> Box2D ����ת��
inline b2Vec2 glmToBox2D(const glm::vec2& v) {
    return b2Vec2(v.x, v.y);
}

// Box2D -> GLM ����ת��
inline glm::vec2 box2DToGlm(const b2Vec2& v) {
    return glm::vec2(v.x, v.y);
}

// GLM -> PhysX ����ת���������� PhysX �� PxVec3��
inline physx::PxVec3 glmToPhysX(const glm::vec3& v) {
    return physx::PxVec3(v.x, v.y, v.z);
}