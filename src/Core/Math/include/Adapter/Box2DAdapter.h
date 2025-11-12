#pragma once
#include "box2d/box2d.h"
#include "AYMathType.h"

namespace AYMath::Adapter {
    // Box2D → Engine
    inline Vector3 fromBox2D(const b2Vec2& v) {
        return Vector3(v.x, v.y, 0.0f);
    }
    
    inline Quaternion fromBox2D(float angle) {
        return glm::angleAxis(angle, Vector3(0, 0, 1));
    }
    
    // Engine → Box2D
    inline b2Vec2 toBox2D(const Vector2& v) {
        return b2Vec2(v.x, v.y);
    }
    
    inline float toBox2D(const Quaternion& q) {
        return glm::roll(q);  // 提取绕Z轴旋转角
    }
}