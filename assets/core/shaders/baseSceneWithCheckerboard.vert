#version 460 core

in vec3 inPosition; 
in vec2 inTexcoords; 
in vec3 inNormal;

out vec2 TexCoords; 
out vec3 Normal; 
out vec3 FragPos; 

uniform float PI;
uniform float time;
uniform mat4 mv_matrix;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj_matrix;
uniform vec3 m_scale;

void main(void) 
{  
	vec4 pos = vec4(inPosition * m_scale,1.0);
	gl_Position = proj_matrix * view * model * pos; 
	TexCoords = inTexcoords * m_scale.xy;
	Normal = vec3(model * vec4(inNormal,1.0));
	FragPos = vec3(model * pos);
};