#version 460 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;
uniform sampler2D equirectangularMap;
uniform int skyboxType; // 0=cubemap, 1=equirectangular, 2=hemisphere

const vec3 skyColor = vec3(0.1, 0.3, 0.8);
const vec3 horizonColor = vec3(0.7, 0.8, 0.9);
const vec3 groundColor = vec3(0.2, 0.2, 0.2);

vec2 sampleSphericalMap(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y)); // 计算球面坐标到 UV 的映射
    uv *= vec2(0.1591, 0.3183); // 1/2pi, 1/pi 归一化到 [0,1] 范围
    uv += 0.5; // 中心化到 [0.5,1.5]

    uv.y -= 0.15;
    uv.y = 1.0 - uv.y;

    return uv;
}

void main() 
{
    if (skyboxType == 0) { // Cubemap
        FragColor = texture(skybox, TexCoords);
    } 
    else if (skyboxType == 1) { // Equirectangular
        vec2 uv = sampleSphericalMap(normalize(TexCoords));
        FragColor = texture(equirectangularMap, uv);
    }
    else { // Hemisphere
        float factor = clamp(TexCoords.y, 0.0, 1.0);
        FragColor = vec4(mix(horizonColor, skyColor, factor), 1.0);
        
        // 添加简单的太阳
        vec3 sunDir = normalize(vec3(0.5, 0.8, 0.6));
        float sun = max(dot(normalize(TexCoords), sunDir), 0.0);
        sun = pow(sun, 32.0);
        FragColor.rgb += vec3(sun * 0.8);
    }
}