#version 460 core
mat4 buildTranslate(float x, float y, float z)
{
	mat4 trans = mat4(1.0, 0.0, 0.0, 0.0,
					0.0, 1.0, 0.0, 0.0,
					0.0, 0.0, 1.0, 0.0,
					x, y, z, 1.0
	);
	return trans;
}

mat4 buildRotateX(float rad)
{
	mat4 rotX = mat4(1.0, 0.0, 0.0, 0.0,
					0.0, cos(rad), -sin(rad), 0.0,
					0.0, sin(rad), cos(rad), 0.0,
					0.0, 0.0, 0.0, 1.0
	);
	return rotX;
}

mat4 buildRotateY(float rad)
{
	mat4 rotY = mat4(cos(rad), 0.0, sin(rad), 0.0,
					0.0, 1.0, 0.0, 0.0,
					-sin(rad), 0.0, cos(rad), 0.0,
					0.0, 0.0, 0.0, 1.0
	);
	return rotY;
}
mat4 buildRotateZ(float rad)
{
	mat4 rotZ = mat4(cos(rad), -sin(rad), 0.0, 0.0,
					sin(rad), cos(rad), 0.0, 0.0,
					0.0, 0.0, 1.0, 0.0,
					0.0, 0.0, 0.0, 1.0
	);
	return rotZ;
}

mat4 buildScale(float x, float y, float z)
{
	mat4 scale = mat4(x, 0.0, 0.0, 0.0,
					0.0, y, 0.0, 0.0,
					0.0, 0.0, z, 0.0,
					0.0, 0.0, 0.0, 1.0
	);
	return scale;
}

in vec3 screenVertices; 

out vec3 vColor; 
out vec3 vPos; 
out vec2 vUV; 
out vec2 TexCoords; 

uniform float PI;
uniform float time;
uniform mat4 mv_matrix;
uniform mat4 proj_matrix;

void main(void) 
{  

	gl_Position =  vec4(screenVertices,1.0); 
};