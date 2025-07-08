#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
        
out vec2 TexCoord;
        
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec2 uvOffset;
uniform vec2 uvSize;
uniform float flipH;
uniform float flipW;
        
void main() {
    vec2 texCoord = aTexCoord;
    if(flipH < 0.0) texCoord.x = 1.0 - texCoord.x; 
    if(flipW < 0.0) texCoord.y = 1.0 - texCoord.y; 

    TexCoord = uvOffset + texCoord * uvSize;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}