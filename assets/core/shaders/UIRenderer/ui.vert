#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec4 aColor;
layout (location = 3) in vec3 aNormal;
    
out vec2 TexCoord;
out vec4 Color;
    
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

    
void main() 
{
    vec2 texCoord = aTexCoord;
    TexCoord = texCoord;
    Color = aColor;
    gl_Position = u_projection * u_view * u_model * vec4(aPos, 1.0);
}