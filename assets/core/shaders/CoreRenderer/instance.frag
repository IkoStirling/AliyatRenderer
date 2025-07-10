#version 460 core
in vec4 v_Color;
out vec4 FragColor;

uniform float u_Metallic;
uniform float u_Roughness;
void main() 
{
    FragColor = v_Color;
    FragColor.rgb *= (1.0 - u_Metallic * 0.5);
}