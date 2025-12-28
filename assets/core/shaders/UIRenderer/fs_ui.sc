$input v_texcoord0, v_color0, v_size_pixel, v_edge_pixel, v_corner_radius_pixel
#include <bgfx_shader.sh>
#include <bgfx_compute.sh>

uniform vec4 u_color;
SAMPLER2D(s_texColor, 0);

uniform vec4 u_borderColor; // 边框颜色(RGBA)
uniform vec4 u_borderParams; // x: 边框宽度(比例), y: 边框抗锯齿, z: 是否启用边框, w: 保留
uniform vec4 u_cornerParams; // x: 圆角半径(宽度x的比例), w: 长宽比(hight/width), z: 抗锯齿范围(0.005f最合适), w: 是否使用纹理

uniform vec4 u_uiElementColor;  // 背景色
uniform vec4 u_uiStrokeColor;   // 描边色
uniform sampler2D u_texture;   // 可选纹理（字体/图标）
uniform bool u_useTexture;    // 是否启用纹理

// 核心函数：计算“当前像素”到圆角矩形的“像素级SDF距离”（解决描边失真的核心）
float rounded_rect_sdf(vec2 texcoord, vec2 size_pixel, float corner_radius_pixel) {
    // 步骤1：将归一化纹理坐标（[0,1]）转换为“像素级坐标”（以UI中心为原点）
    // 比如UI是200x100像素，texcoord=(0,0)→像素坐标=(-100,-50)，texcoord=(1,1)→(100,50)
    vec2 pixel_coord = (texcoord - 0.5) * size_pixel;
    
    // 步骤2：限制圆角半径（避免圆角超过UI最小边长的一半，比如100x50的UI，最大圆角只能25）
    float max_radius = min(size_pixel.x, size_pixel.y) * 0.5;
    float radius = clamp(corner_radius_pixel, 0.0, max_radius);
    
    // 步骤3：计算SDF距离（像素级）
    // abs(pixel_coord)：转换为第一象限（对称）
    // 减去（UI半尺寸 - 圆角半径）：得到到“圆角内矩形”的距离
    vec2 dist = abs(pixel_coord) - (size_pixel * 0.5 - radius);
    // length(max(dist, 0.0))：如果在圆角区域外，计算欧氏距离；否则为0
    // 减去radius：最终得到“像素级SDF距离”（负值=在UI内，正值=在UI外）
    float sdf = length(max(dist, 0.0)) - radius;
    
    return sdf;
}

void main() {
    // 1. 计算当前像素到UI的像素级SDF距离
    float sdf = rounded_rect_sdf(v_texcoord0, v_size_pixel, v_corner_radius_pixel);
    
    // 2. 抗锯齿范围（1像素，适配像素密度，避免不同屏幕锯齿差异）
    float aa_range = 1.0 / u_cornerParams.w;
    
    // 3. 计算背景色权重（smoothstep实现抗锯齿）
    // sdf < -aa_range：完全在UI内，权重=1（实心）
    // sdf > aa_range：完全在UI外，权重=0（透明）
    // 中间范围：渐变过渡（抗锯齿）
    float bg_alpha = smoothstep(aa_range, -aa_range, sdf);
    
    // 4. 计算描边权重（核心：像素级精确描边）
    // 描边范围：sdf ∈ [0, v_edge_pixel]（UI边缘外0~描边宽度的像素）
    // 用两个smoothstep相减，得到描边的渐变权重
    float stroke_alpha = smoothstep(aa_range, -aa_range, sdf) - smoothstep(v_edge_pixel + aa_range, v_edge_pixel - aa_range, sdf);
    
    // 5. 纹理混合（可选：字体、图标）
    vec4 tex_color = vec4(1.0);
    if (u_useTexture) {
        tex_color = texture2D(u_texture, v_texcoord0);
        tex_color.a = tex_color.r;  // 字体纹理通常是单通道（r通道存亮度）
    }
    
    // 6. 混合背景、描边、纹理（按权重叠加）
    vec4 final_color = vec4(0.0);
    // 先叠加描边
    final_color = mix(final_color, u_uiStrokeColor * tex_color, stroke_alpha * u_uiStrokeColor.a);
    // 再叠加背景（注意：背景权重要减去描边权重，避免重叠）
    final_color = mix(final_color, u_uiElementColor * tex_color, bg_alpha * u_uiElementColor.a * (1.0 - stroke_alpha));
    
    // 7. 透明像素裁剪（减少过度绘制，提升性能）
    if (final_color.a < 0.01) discard;
    
    // 输出最终像素颜色
    gl_FragColor = final_color;
}
