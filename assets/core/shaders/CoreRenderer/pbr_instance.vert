#version 460 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in mat4 instanceMatrix;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3 v_Position;
out vec3 v_Normal;
out vec2 v_TexCoord;

void main() {
    mat4 modelMatrix = model * instanceMatrix;
    vec4 worldPosition = modelMatrix * vec4(aPos, 1.0);
    
    v_Position = worldPosition.xyz;
    v_Normal = mat3(transpose(inverse(modelMatrix))) * aNormal;
    v_TexCoord = aTexCoord;
    
    gl_Position = projection * view * worldPosition;
}