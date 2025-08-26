#pragma once
#include "BasePhy/IAYPhysicsBody.h"

struct STRaycastResult
{
    IAYPhysicsBody* body;      // 击中的物理体
    glm::vec3 point = glm::vec3(0);           // 击中点（世界坐标）
    glm::vec3 normal = glm::vec3(0);          // 击中点的法线
    float fraction = -1.f;            // 击中点沿射线的比例（0到1之间）
};