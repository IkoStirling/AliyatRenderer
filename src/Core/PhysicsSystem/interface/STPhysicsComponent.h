#pragma once
#include "STTransform.h"
namespace ayt::engine::physics
{
    using EntityID = uint32_t;
    enum class WorldType { AY2D, AY3D };
    enum class BodyType { Static, Dynamic, Kinematic };
    struct STPhysicsComponent {
        EntityID bodyID; // 关联的物理实体ID
        WorldType worldType;
        BodyType bodyType;
        //std::vector<ColliderID> colliders;
        bool isSimulated = true;

        // 缓存上次同步的变换
        math::Transform lastTransform;
    };
}