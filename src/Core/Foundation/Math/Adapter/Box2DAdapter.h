#pragma once
#include "box2d/box2d.h"
#include "AYMathType.h"

namespace ayt::engine::math::adapter {
    // Box2D → Engine
    inline Vector2 fromBox2DV2(const b2Vec2& v) {
        return Vector2(v.x, v.y);
    }

    inline Vector3 fromBox2DV3(const b2Vec2& v) {
        return Vector3(v.x, v.y, 0.0f);
    }
    
    inline Quaternion fromBox2DRoll(float angle) {
        return glm::angleAxis(angle, Vector3(0, 0, 1));
    }
    
    // Engine → Box2D
    inline b2Vec2 toBox2DV2(const Vector2& v) {
        return b2Vec2(v.x, v.y);
    }

    inline b2Vec2 toBox2DV3(const Vector3& v) {
        return b2Vec2(v.x, v.y);
    }
    
    inline float toBox2DRoll(const Quaternion& q) {
        return glm::roll(q);  // 提取绕Z轴旋转角
    }

    inline b2Rot toBox2DRot(float degree)
    {
        float radians = glm::radians(degree);
        float c = glm::cos(radians);
        float s = glm::sin(radians);

        return b2Rot(c, s);
    }

}