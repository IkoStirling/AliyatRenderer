$input v_texcoord0, v_color0

#include <bgfx_shader.sh>
#include <bgfx_compute.sh>

uniform vec4 u_color;
SAMPLER2D(s_texColor, 0);

uniform vec4 u_borderColor; // 边框颜色(RGBA)
uniform vec4 u_borderParams; // x: 边框宽度(比例), y: 边框抗锯齿, z: 是否启用边框, w: 保留
uniform vec4 u_cornerParams; // x: 圆角半径(宽度x的比例), w: 长宽比(hight/width), z: 抗锯齿范围(0.005f最合适), w: 是否使用纹理

float calculateRoundedCornerAlpha(vec2 uv, float radius, float ratio, float antiAlias, float scale) {
    vec2 centeredUV = (uv - 0.5) / scale + 0.5;
    
    // 只处理在缩放后图形内的像素
    if (centeredUV.x < 0.0 || centeredUV.x > 1.0 || centeredUV.y < 0.0 || centeredUV.y > 1.0) {
        return 0.0;
    }
    vec2 p          = abs(step(0.5, centeredUV) - centeredUV) * scale;
    
    float horizontal = 1.0 - smoothstep(radius - antiAlias, radius + antiAlias, p.x);
    float vertical  = 1.0 - smoothstep(radius - antiAlias, radius + antiAlias, p.y * ratio);
    float circle    = smoothstep(radius - antiAlias, radius + antiAlias, 
                   length(vec2(p.x - radius, p.y * ratio - radius)));
    
    return 1.0 - horizontal * vertical * circle;
}

void main() {
    float radius    = u_cornerParams.x;
    float ratio     = u_cornerParams.y;
    float aa        = u_cornerParams.z * ratio;
    float useTexture = u_cornerParams.w;

    float borderWidth = u_borderParams.x;
    float borderAA  = u_borderParams.y;
    bool useBorder  = u_borderParams.z > 0.5;

    vec4 baseColor  = step(0.5, useTexture)* texture2D(s_texColor, v_texcoord0) + step(useTexture, 0.5) * vec4(1,1,1,1);
    vec4 finalColor = baseColor * v_color0 * u_color;

    float outerAlpha = calculateRoundedCornerAlpha(v_texcoord0, radius, ratio, aa, 1.0);
    float innerAlpha = calculateRoundedCornerAlpha(v_texcoord0, radius * 0.8, ratio, aa, 1.0 - borderWidth);
    float borderAlpha = outerAlpha - innerAlpha;

    //finalColor      *= outerAlpha;


    if (useBorder && borderAlpha > 0.01) {
        // 边框抗锯齿
        float borderSmooth = smoothstep(0.0, borderAA, borderAlpha);
        finalColor = mix(finalColor, u_borderColor, borderSmooth);
    } else {
        finalColor.a *= outerAlpha;
    }

    if (finalColor.a < 0.01) discard;

    gl_FragColor    = finalColor;
}

// 关于圆角逻辑迭代
// 首先判断到边界距离是否大于半径， 然后判断像素到边界距离需要小于半径
// float corner = step(radius, p.x) + step(radius, p.y) + step(length(vec2(p.x - radius, p.y - radius)), radius);
// finalColor   *= step(corner, 0.99);
// float corner = smoothstep(radius-aa,radius+aa, p.x) + smoothstep(radius-aa,radius+aa, p.y * ratio) + 1.0 - smoothstep(radius-aa,radius+aa, length(vec2(p.x - radius, p.y * ratio - radius)));
// finalColor   *= clamp(0.0,1.0, corner);
// float corner = (1.0 - smoothstep(radius-aa,radius+aa, p.x)) * (1.0 - smoothstep(radius-aa,radius+aa, p.y * ratio)) * smoothstep(radius-aa,radius+aa, length(vec2(p.x - radius, p.y * ratio - radius)));
// finalColor   *= 1.0 - corner;

