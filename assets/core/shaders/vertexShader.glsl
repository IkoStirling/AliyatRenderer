#version 460 
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

in vec3 aPos; 
layout(location=2) in vec3 aColor; 
layout(location=3) in vec2 aUV; 

out vec3 vColor; 
out vec3 vPos; 
out vec2 vUV; 

uniform float PI;
uniform float time;
uniform mat4 mv_matrix;
uniform mat4 proj_matrix;

void main(void) 
{  
	vec3 pos = vec3(aPos.x + 0.5 * sin(time),aPos.y,aPos.z);
	vec4 mid = vec4(pos,1.0) * buildRotateZ(0.707) * buildRotateX(0.707);
	
	vec4 mid2 = vec4(aPos,1.0);
	vec4 mid3 = buildRotateY(time * 2.0) * mid2;
	vec4 position = buildTranslate(0.0,sin(time)*2.0, 0.0) * mid3;
	position = buildTranslate(0.0,gl_InstanceID*1.5,0.0) * position;

	gl_Position =  proj_matrix * mv_matrix *  position; 
	vColor = aColor; 
	vPos = aPos.xyz; 
	vUV = aUV;
};