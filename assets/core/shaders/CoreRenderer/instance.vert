#version 460 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;
layout(location=3) in mat4 instanceMatrix; // 使用位置3-6

//out vec4 v_Color;
out vec2 v_TexCoord;
out vec3 v_Normal;
out vec3 v_FragPos;

uniform mat4 u_Projection;
uniform mat4 u_View;

void main() 
{
    v_FragPos = vec3(instanceMatrix *  vec4(aPos, 1.0));
    v_Normal = mat3(transpose(inverse(instanceMatrix))) * aNormal; // 正确处理非均匀缩放
    v_TexCoord = aTexCoord;
    gl_Position = u_Projection * u_View * vec4(v_FragPos, 1.0);
}