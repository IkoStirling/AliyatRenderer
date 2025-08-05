#pragma once
#include "AYECSEngine.h"
#include "STPhysicsComponent.h"
#include <box2d/b2_math.h> 
#include <foundation/PxVec3.h>
#include <iostream>
#include <sstream>
#include <glm/glm.hpp>
#include <atomic>
#include <functional>

// GLM -> Box2D 向量转换
const float PIXELS_PER_METER = 16.f;

inline b2Vec2 glmToBox2D(const glm::vec2& v) {
    return b2Vec2(v.x, v.y);
}

// Box2D -> GLM 向量转换
inline glm::vec2 box2DToGlm(const b2Vec2& v) {
    return glm::vec2(v.x, v.y);
}

inline glm::vec2 box2DToEngine(const b2Vec2& v) {
    return glm::vec2(v.x * PIXELS_PER_METER, v.y * PIXELS_PER_METER);
}

inline glm::vec2 glmToEngine(const glm::vec2& v) {
    return v * PIXELS_PER_METER;
}

// GLM -> PhysX 向量转换（需适配 PhysX 的 PxVec3）
inline physx::PxVec3 glmToPhysX(const glm::vec3& v) {
    return physx::PxVec3(v.x, v.y, v.z);
}