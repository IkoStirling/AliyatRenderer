#version 460 core

in vec3 v_Position;
in vec3 v_Normal;
in vec2 v_TexCoord;

out vec4 FragColor;

// 材质参数
uniform vec4 u_BaseColor;
uniform float u_Metallic;
uniform float u_Roughness;
uniform float u_AO;
uniform float u_Reflectance;
uniform vec3 u_EmissiveColor;
uniform float u_EmissiveIntensity;

// 纹理
uniform sampler2D u_AlbedoTexture;
uniform sampler2D u_NormalTexture;
uniform sampler2D u_MetallicTexture;
uniform sampler2D u_RoughnessTexture;
uniform sampler2D u_AOTexture;
uniform sampler2D u_EmissiveTexture;

// 纹理使用标志
uniform bool u_UseAlbedoTexture;
uniform bool u_UseNormalTexture;
uniform bool u_UseMetallicTexture;
uniform bool u_UseRoughnessTexture;
uniform bool u_UseAOTexture;
uniform bool u_UseEmissiveTexture;

// 光照参数
uniform vec3 u_LightPositions[4];
uniform vec3 u_LightColors[4];
uniform vec3 u_CameraPos;

const float PI = 3.14159265359;

// 法线分布函数 (Trowbridge-Reitz GGX)
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

// 几何函数 (Schlick GGX)
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

// 几何函数 (Smith)
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// Fresnel方程 (Schlick近似)
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    // 从纹理或uniform获取材质属性
    vec3 albedo = u_UseAlbedoTexture ? texture(u_AlbedoTexture, v_TexCoord).rgb : u_BaseColor.rgb;
    float metallic = u_UseMetallicTexture ? texture(u_MetallicTexture, v_TexCoord).r : u_Metallic;
    float roughness = u_UseRoughnessTexture ? texture(u_RoughnessTexture, v_TexCoord).r : u_Roughness;
    float ao = u_UseAOTexture ? texture(u_AOTexture, v_TexCoord).r : u_AO;
    vec3 emissive = u_UseEmissiveTexture ? texture(u_EmissiveTexture, v_TexCoord).rgb : u_EmissiveColor;
    
    // 输入数据
    vec3 N = normalize(v_Normal);
    if(u_UseNormalTexture) {
        // 从法线贴图获取法线 (需要TBN矩阵)
        N = texture(u_NormalTexture, v_TexCoord).rgb;
        N = normalize(N * 2.0 - 1.0);
    }
    
    vec3 V = normalize(u_CameraPos - v_Position);
    
    // 计算反射率
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
    
    // 反射方程
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 4; ++i) {
        // 计算每个光源的贡献
        vec3 L = normalize(u_LightPositions[i] - v_Position);
        vec3 H = normalize(V + L);
        float distance = length(u_LightPositions[i] - v_Position);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = u_LightColors[i] * attenuation;
        
        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);   
        float G = GeometrySmith(N, V, L, roughness);    
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
        
        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;
        
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;
        
        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }
    
    // 环境光照 (简化版)
    vec3 ambient = vec3(0.03) * albedo * ao;
    
    // 自发光
    vec3 emission = emissive * u_EmissiveIntensity;
    
    vec3 color = ambient + Lo + emission;
    
    // HDR色调映射
    color = color / (color + vec3(1.0));
    // gamma校正
    color = pow(color, vec3(1.0/2.2)); 
    
    FragColor = vec4(color, u_BaseColor.a);
}