$input a_position, a_texcoord0, a_color0
$output v_texcoord0, v_color0, v_size_pixel, v_edge_pixel, v_corner_radius_pixel

#include <bgfx_shader.sh>

uniform vec4 u_uiParams;        // x:屏幕宽(像素), y:屏幕高(像素), z:像素密度, w:opacity
uniform vec4 u_uiElementParams; // x: 描边宽度 y: 圆角半径
uniform vec4 u_uiElementColor;  // background color
uniform vec4 u_uiStrokeColor;   // stroke color

void main() {
    // 1. 计算UI最终屏幕坐标（正交投影，无透视，适合2D UI）
    // u_modelViewProj：BGFX提供的模型-视图-投影矩阵，包含UI的位置、缩放信息
    gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));

    // 2. 传递纹理坐标（归一化[0,1]，不管UI多大，纹理坐标都在0~1范围）
    v_texcoord0 = a_texcoord0;

    // 3. 核心：计算UI元素的“像素级尺寸”（解决描边失真的关键前提）
    // 从模型矩阵中提取缩放信息（u_modelViewProj[0]是X轴缩放，[1]是Y轴缩放）
    vec2 element_scale = vec2(length(u_modelViewProj[0].xyz), length(u_modelViewProj[1].xyz));
    // 转换为像素尺寸：屏幕尺寸*缩放*0.5 / 像素密度（适配不同屏幕）
    v_size_pixel = element_scale * 0.5 * vec2(u_uiElementParams.x, u_uiElementParams.y) / u_uiElementParams.z;
    
    // 4. 传递描边宽度和圆角半径（像素级，直接给片段着色器用）
    v_edge_pixel = u_uiElementParams.x;
    v_corner_radius_pixel = u_uiElementParams.y;
}
