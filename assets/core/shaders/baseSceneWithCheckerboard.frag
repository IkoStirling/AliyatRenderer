#version 460 

in vec3 FragPos; 
in vec3 Normal;
in vec2 TexCoords; 
out vec4 color; 

uniform vec3 lightColor;
uniform vec3 lightDir;
uniform vec3 cameraPos;
uniform float shininess;
uniform float abientStrength;


uniform float PI;
uniform float time;
uniform vec2 iResolution;
uniform sampler2D texture1;
layout(binding=0) uniform sampler2D samp;

vec3 ambientLight(vec3 lightColor, float strength)
{
	return lightColor* strength;
}

vec3 diffuseLight(vec3 lightColor, vec3 lightDir, vec3 normal)
{
	float diff = max(dot(normal,-lightDir),0.0);
	return lightColor * diff;
}

vec3 specularLight(vec3 lightColor, vec3 lightDir, vec3 normal, vec3 viewDir, float shininess)
{
	vec3 reflectDir = reflect(lightDir, normal);
	float spec = pow(max(dot(viewDir,reflectDir),0.0), shininess);
	return lightColor * spec;
}

vec3 calculateLight(vec3 lightColor, vec3 lightDir, vec3 normal, vec3 viewDir, float shininess, float ambientStrength)
{
	vec3 ambient = ambientLight(lightColor,ambientStrength);
	vec3 diffuse = diffuseLight(lightColor, lightDir, normal);
	vec3 specular = specularLight(lightColor,lightDir,normal,viewDir,shininess);
	return ambient + diffuse + specular;
}

void main(void) 
{
	vec3 viewDir = normalize(cameraPos - FragPos);
	vec3 normal = normalize(Normal);
	vec3 lightDir = normalize(lightDir);
	vec3 result = calculateLight(lightColor,lightDir,normal,viewDir,shininess,abientStrength);
	color = texture(samp,TexCoords) * vec4(result,1.0);
	//color = vec4(1.0,1.0,1.0,1.0);
};