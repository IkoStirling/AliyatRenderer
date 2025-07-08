#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
    
out vec2 TexCoord;
    
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float flipH;
uniform float flipW;
    
void main() 
{
    vec2 texCoord = aTexCoord;
    if(flipH < 0.0) {
        texCoord.x = 1.0 - texCoord.x; // 水平翻转UV坐标
    }
    if(flipW < 0.0) {
        texCoord.y = 1.0 - texCoord.y; // 垂直翻转UV坐标
    }
    TexCoord = texCoord;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}