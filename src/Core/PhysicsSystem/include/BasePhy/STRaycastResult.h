#pragma once
#include "BasePhy/IAYPhysicsBody.h"
namespace ayt::engine::physics
{
    struct RaycastResult
    {
        IPhysicsBody* body;      // 击中的物理体
        math::Vector3 point = math::Vector3(0);           // 击中点（世界坐标）
        math::Vector3 normal = math::Vector3(0);          // 击中点的法线
        float fraction = -1.f;            // 击中点沿射线的比例（0到1之间）
    };
}