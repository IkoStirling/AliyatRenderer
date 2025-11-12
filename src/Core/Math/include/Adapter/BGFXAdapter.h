#pragma once
#include "bgfx/bgfx.h"
#include "AYMathType.h"

namespace AYMath::Adapter {
    // BGFX使用行优先矩阵，需要转换
    inline void toBGFX(const Matrix4& m, float* out) {
        // BGFX期望的格式
        memcpy(out, glm::value_ptr(glm::transpose(m)), sizeof(Matrix4));
    }
    
    // BGFX → Engine (如果需要反向转换)
    inline Matrix4 fromBGFX(const float* m) {
        Matrix4 mat = glm::make_mat4(m);
        return glm::transpose(mat);
    }
}