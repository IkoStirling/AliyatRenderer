#pragma once
#include "PxPhysicsAPI.h"
#include "AYMathType.h"

namespace ayt::engine::math::adapter {
    // PhysX → Engine
    inline Vector3 fromPhysX(const physx::PxVec3& v) {
        return Vector3(v.x, v.y, v.z);
    }
    
    inline Quaternion fromPhysX(const physx::PxQuat& q) {
        return Quaternion(q.w, q.x, q.y, q.z);
    }
    
    inline Matrix4 fromPhysX(const physx::PxTransform& t) {
        Vector3 pos = fromPhysX(t.p);
        Quaternion rot = fromPhysX(t.q);
        return glm::translate(Matrix4(1.0f), pos) * glm::mat4_cast(rot);
    }
    
    // Engine → PhysX
    inline physx::PxVec3 toPhysX(const Vector3& v) {
        return physx::PxVec3(v.x, v.y, v.z);
    }
    
    inline physx::PxQuat toPhysX(const Quaternion& q) {
        return physx::PxQuat(q.x, q.y, q.z, q.w);
    }
}