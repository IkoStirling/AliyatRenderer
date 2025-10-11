$input a_position, a_texcoord0, a_color0
$output v_texcoord0, v_color0

#include <bgfx_shader.sh>

uniform vec4 u_uiParams; // x: opacity, y: time, zw: unused

void main() {
    vec3 position = a_position;

    float hoverEffect = sin(u_uiParams.y) * 0.1 * a_color0.a;
    position += vec3(hoverEffect,hoverEffect,0.0);

    gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
    v_texcoord0 = vec2(a_texcoord0.x, 1.0 - a_texcoord0.y);

    v_color0 = a_color0 * u_uiParams.x;
}
