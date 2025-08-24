#version 460 core
layout(std430, binding = 0) buffer LightData {
    vec4 directionalDirections[4];
    vec4 directionalColors[4];
    
    vec4 pointPositions[16];
    vec4 pointColors[16];
    vec4 pointParams[16];
    
    vec4 spotPositions[8];
    vec4 spotDirections[8];
    vec4 spotColors[8];
    vec4 spotParams[8];
    vec4 spotAttenuation[8];
    
    ivec4 lightCounts;
} lights;

uniform vec3 u_ViewPos;
uniform vec4 u_BaseColor;
uniform float u_Metallic;
uniform float u_Roughness;

uniform sampler2D u_AlbedoTexture;  // 新增：albedo纹理
uniform bool u_UseAlbedoTexture;    // 新增：是否使用albedo纹理标志
uniform sampler2D u_OpacityTexture;      // 透明度贴图
uniform bool u_UseOpacityTexture;        // 是否使用透明度贴图
uniform bool u_DebugMode;        // 是否使用透明度贴图




in vec2 v_TexCoord;
in vec3 v_Normal;
in vec3 v_FragPos;
out vec4 FragColor;


vec3 CalcDirectionalLight(vec4 directionalColors, vec4 directionalDirections, vec3 normal, vec3 viewDir) {
    // 漫反射
    float intensity = directionalColors.a;
    vec3 color = directionalColors.rgb;
    vec3 direction = directionalDirections.xyz;

    vec3 lightDir = normalize(-direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = color * diff * intensity;
    
    // 镜面反射 (Blinn-Phong)
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = color * spec * intensity;
    
    // 简单模拟金属度影响
    float metallicFactor = 1.0 - u_Metallic * 0.5;
    
    return (diffuse + specular) * metallicFactor;
}

// 计算点光源贡献
vec3 CalcPointLight(vec4 pointPositions, vec4 pointColors, vec4 pointParams, vec3 normal, vec3 fragPos, vec3 viewDir) 
{
    vec3 position = pointPositions.xyz;
    vec3 color = pointColors.rgb;
    float intensity = pointParams.x;
    float radius = pointParams.y;
    float constant = pointParams.z;
    float linear = pointParams.w;
    float quadratic = pointColors.w;

    vec3 lightDir = normalize(position - fragPos);
    float dist = length(position - fragPos);
    
    // 衰减计算
    float attenuation = 1.0 / (constant + linear * dist + 
                 quadratic * (dist * dist));
    
    // 漫反射
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = color * diff * intensity * attenuation;
    
    // 镜面反射
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = color * spec * intensity * attenuation;
    
    // 金属度影响
    float metallicFactor = 1.0 - u_Metallic * 0.5;
    
    return (diffuse + specular) * metallicFactor;
}

// 计算聚光灯贡献
vec3 CalcSpotLight(vec4 spotPositions,vec4 spotDirections,vec4 spotColors,vec4 spotParams,vec4 spotAttenuation, vec3 normal, vec3 fragPos, vec3 viewDir) 
{
    vec3 position = spotPositions.xyz;
    vec3 direction = spotDirections.xyz;
    vec3 color = spotColors.rgb;
    float intensity = spotParams.x;
    float cutOff = spotParams.y;
    float outerCutOff = spotParams.z;
    float constant = spotAttenuation.y;
    float linear = spotAttenuation.z;
    float quadratic = spotAttenuation.w;

    vec3 lightDir = normalize(position - fragPos);
    float dist = length(position - fragPos);
    float attenuation = 1.0 / (constant + linear * dist + 
                 quadratic * (dist * dist));
    
    // 聚光强度计算
    float theta = dot(lightDir, normalize(-direction));
    float epsilon = cutOff - outerCutOff;
    float spotIntensity  = clamp((theta - outerCutOff) / epsilon, 0.0, 1.0);
    
    // 漫反射
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = color * diff * intensity * attenuation * spotIntensity;
    
    // 镜面反射
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = color * spec * intensity * attenuation * spotIntensity;
    
    // 金属度影响
    float metallicFactor = 1.0 - u_Metallic * 0.5;
    
    return (diffuse + specular) * metallicFactor;
}



void main() 
{
    vec3 normal = normalize(v_Normal);
    vec3 viewDir = normalize(u_ViewPos - v_FragPos);

    vec3 ambient = vec3(0.1);
    
    // 获取基础颜色：如果使用纹理则从纹理采样，否则使用u_BaseColor
    vec4 baseColor = u_BaseColor;
    if (u_UseAlbedoTexture) {
        baseColor = texture(u_AlbedoTexture, v_TexCoord);
    }

    float alpha = baseColor.a;
    if (u_UseOpacityTexture) {
        // 从透明度贴图中获取Alpha值（通常存储在R通道）
        alpha *= texture(u_OpacityTexture, v_TexCoord).r;
    }

    // 完全透明的像素直接丢弃
    if (alpha < 0.01) {
        discard;
    }

    // 初始化光照结果
    vec3 result = ambient * baseColor.rgb;
    
    int numDirLights = lights.lightCounts.x;
    int numPointLights = lights.lightCounts.y;
    int numSpotLights = lights.lightCounts.z;

    // 处理所有定向光
    for(int i = 0; i < numDirLights; i++) {
        result += CalcDirectionalLight(
            lights.directionalColors[i],
            lights.directionalDirections[i],
            normal, viewDir) * baseColor.rgb;
    }

    // 处理所有点光源
    for(int i = 0; i < numPointLights; i++) {
        result += CalcPointLight(lights.pointPositions[i],
            lights.pointColors[i],
            lights.pointParams[i],
            normal, v_FragPos, viewDir) * baseColor.rgb;
    }
    
    // 处理所有聚光灯
    for(int i = 0; i < numSpotLights; i++) {
        result += CalcSpotLight(lights.spotPositions[i],
            lights.spotDirections[i],
            lights.spotColors[i],
            lights.spotParams[i],
            lights.spotAttenuation[i],
            normal, v_FragPos, viewDir) * baseColor.rgb;
    }
    
    // 粗糙度简单影响高光范围
    float roughnessFactor = 1.0 - u_Roughness * 0.3;
    result *= roughnessFactor;
    
    FragColor = vec4(result, alpha);

    if (u_DebugMode) {
    FragColor = vec4(vec3(gl_FragCoord.z), 1.0); // 可视化深度
    }
}