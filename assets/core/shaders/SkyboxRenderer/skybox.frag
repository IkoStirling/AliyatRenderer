#version 460 core
out vec4 FragColor;

in vec3 TexCoords;
uniform samplerCube skybox;
uniform sampler2D equirectangularMap;
uniform vec3 u_cameraPos;
uniform int skyboxType; // 0=cubemap, 1=equirectangular, 2=hemisphere, 3=galaxy


vec2 sampleSphericalMap(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y)); // 计算球面坐标到 UV 的映射
    uv *= vec2(0.1591, 0.3183); // 1/2pi, 1/pi 归一化到 [0,1] 范围
    uv += 0.5; // 中心化到 [0.5,1.5]

    uv.y -= 0.15;
    uv.y = 1.0 - uv.y;

    return uv;
}

uniform float u_time;   // 用于动态效果

// 颜色常量
//const vec3 skyColor = vec3(0.1, 0.3, 0.8);
const vec3 skyColor = vec3(0.01, 0.02, 0.05);  // 深太空背景
const vec3 horizonColor = vec3(0.7, 0.8, 0.9);
const vec3 groundColor = vec3(0.2, 0.2, 0.2);
const vec3 starColor = vec3(0.8, 0.9, 1.0);
const vec3 galaxyCoreColor = vec3(0.15, 0.1, 0.2);   // 银河颜色
const vec3 nebulaColor1 = vec3(0.15, 0.05, 0.4);   // 星云颜色1
const vec3 nebulaColor2 = vec3(0.01, 0.02, 0.05);   // 星云颜色2

float rand(vec3 seed) {
    return fract(sin(dot(seed, vec3(12.9898, 78.233, 45.543))) * 43758.5453);
}

float hash(vec3 p) {
    p = fract(p * 0.3183099 + 0.1);
    p *= 17.0;
    float h = fract(p.x * p.y * p.z * (p.x + p.y + p.z));
    h = fract(h * h * 127.0);  // 更平滑的分布
    return h;
}

float hash1(vec3 p) {
    // 加入一个固定偏移量，避免 p = (0,0,0) 时输出恒定值
    p += vec3(0.123, 0.456, 0.789);
    
    // 使用经典噪声混合算法
    p = fract(p * 0.3183099 + 0.1);
    p *= 17.0;
    
    // 更复杂的混合，避免周期性重复
    float h = fract(p.x * p.y * p.z * (p.x + p.y + p.z));
    h = fract(h * 13.0 + h * 0.1);  // 进一步扰乱
    
    return h;
}

float noise(vec3 p) {
    vec3 i = floor(p);
    vec3 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    
    float a = hash(i + vec3(0.0, 0.0, 0.0));
    float b = hash(i + vec3(1.0, 0.0, 0.0));
    float c = hash(i + vec3(0.0, 1.0, 0.0));
    float d = hash(i + vec3(1.0, 1.0, 0.0));
    float e = hash(i + vec3(0.0, 0.0, 1.0));
    float f_val = hash(i + vec3(1.0, 0.0, 1.0));
    float g = hash(i + vec3(0.0, 1.0, 1.0));
    float h = hash(i + vec3(1.0, 1.0, 1.0));
    
    float x1 = mix(a, b, f.x);
    float x2 = mix(c, d, f.x);
    float y1 = mix(x1, x2, f.y);
    
    float x3 = mix(e, f_val, f.x);
    float x4 = mix(g, h, f.x);
    float y2 = mix(x3, x4, f.y);
    
    return mix(y1, y2, f.z);
}

float gradientNoise(vec3 p) {
    vec3 i = floor(p);
    vec3 f = fract(p);
    
    // 立方体8个顶点的哈希值
    float a = hash(i + vec3(0.0));
    float b = hash(i + vec3(1.0, 0.0, 0.0));
    float c = hash(i + vec3(0.0, 1.0, 0.0));
    float d = hash(i + vec3(1.0, 1.0, 0.0));
    float e = hash(i + vec3(0.0, 0.0, 1.0));
    float f_val = hash(i + vec3(1.0, 0.0, 1.0));
    float g = hash(i + vec3(0.0, 1.0, 1.0));
    float h_val = hash(i + vec3(1.0, 1.0, 1.0));
    
    // 三线性插值
    vec3 u = f * f * (3.0 - 2.0 * f);
    return mix(
        mix(mix(a, b, u.x), mix(c, d, u.x), u.y),
        mix(mix(e, f_val, u.x), mix(g, h_val, u.x), u.y),
        u.z);
}

float volumeNoise(vec3 p, float scale, float time) {
    p *= scale;
    p.x += time * 0.03;
    p.z += time * 0.02;
    
    float n = 0.0;
    float a = 0.5;
    
    for (int i = 0; i < 3; i++) {
        n += gradientNoise(p) * a;
        p *= 2.0;
        a *= 0.5;
    }
    
    return n;
}

float fbm(vec3 p) {
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;

    for(int i = 0; i < 4; i++) {
        value += amplitude * gradientNoise(p * frequency);
        frequency *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}

// 生成星点
float generateStars(vec3 worldPos) {
    vec3 seed = floor(worldPos * 1000.0);

    float starValue = hash(seed);

    // 判断是否有星星（高亮度值）
    float starThreshold = 0.997;
    float hasStar = smoothstep(starThreshold - 0.002, starThreshold + 0.002, starValue);

    // 每颗星星有不同的“大小” --> 控制亮区的范围宽度
    float sizeVariation = hash(seed * 100.0) * 0.5 + 0.5; // [0.02 ~ 0.07]

    // 映射到 smoothstep 的窗口，比如 [1.0 - sizeVariation*10, 1.0]
    float threshold_min = 1.0 - sizeVariation * 10.0; // 如 0.93 ~ 0.98
    float threshold_max = 1.0;

    float stars = hasStar * smoothstep(threshold_min, threshold_max, starValue);

    float twinkle = sin(u_time * (1.0 + rand(seed) * 3.0) * 0.5 + 0.5);
    stars *= twinkle * 0.5 + 0.5;
        
    return stars;
}

// 生成银河带
float generateGalaxy(vec3 dir) {
    float galaxyMask = 1.0 - smoothstep(0.0, 0.3, abs(dir.y)); // 银河主要在赤道附近

    // 银河中心方向 (假设银河在 XZ 平面，朝向某个方向，比如 -Y)
    vec3 galacticCenter = normalize(vec3(0.0, -0.2, 1.0));
    float galacticAngle = dot(normalize(dir), galacticCenter); // 越接近1表示越朝向银河中心
    float coreBrightness = 1.0 - smoothstep(0.0, 0.8, galacticAngle); // 中心更亮

    // 使用 FBM 或改进的 noise 生成细节
    vec2 noiseInput = dir.xz * 3.0 + vec2(dir.y * 2.0) + vec2(u_time * 0.01);
    float galaxyNoise = fbm(vec3(noiseInput, u_time * 0.005)); // 使用你改进后的 fbm

    float galaxy = galaxyMask * coreBrightness * galaxyNoise;

    // 可选：增强对比度
    galaxy = smoothstep(0.4, 0.8, galaxy);
    return galaxy;
}

// 生成星云
vec3 generateNebula(vec3 worldPos) {
    // 使用世界坐标而非标准化方向
    vec3 p = worldPos * 0.2; // 调整缩放比例
    
    // 多层体积噪声
    float n1 = volumeNoise(p, 1.0, u_time);
    float n2 = volumeNoise(p + vec3(100.0), 2.0, u_time * 0.7);
    float n3 = volumeNoise(p + vec3(-100.0), 0.5, u_time * 0.3);
    
    // 创建密度场
    float density = n1 * 0.6 + n2 * 0.3 + n3 * 0.1;
    density = smoothstep(0.3, 0.6, density);
    
    // 颜色混合
    vec3 nebulaColor = mix(
        nebulaColor1,
        nebulaColor2,
        volumeNoise(p * 0.5, 1.0, u_time * 0.5)
    );
    
    // 添加核心亮度
    float core = pow(density, 2.0) * 1.5;
    
    return nebulaColor * density * core;
}

// 光线步进体积渲染
vec3 renderVolumeNebula(vec3 rayOrigin, vec3 rayDir) {
    const int steps = 32;
    const float stepSize = 0.05;
    
    vec3 col = vec3(0.0);
    float transmittance = 1.0;
    
    for (int i = 0; i < steps; ++i) {
        vec3 p = rayOrigin + rayDir * (i * stepSize);
        
        // 采样密度场
        float density = volumeNoise(p, 0.5, u_time);
        density = smoothstep(0.2, 0.6, density);
        
        // 颜色计算
        vec3 localColor = mix(nebulaColor1, nebulaColor2, 
                            volumeNoise(p * 2.0, 1.0, u_time * 0.3));
        
        // 光照模型 (简单版本)
        float light = density * 0.5 + 
                     pow(volumeNoise(p * 0.3, 1.0, 0.0), 2.0) * 0.7;
        
        // 体积渲染积分
        float absorption = density * stepSize;
        transmittance *= exp(-absorption);
        
        col += localColor * light * stepSize * transmittance;
        
        if (transmittance < 0.01) break;
    }
    
    return col;
}


void main() 
{
    vec3 dir = normalize(TexCoords);

    if (skyboxType == 0) { // Cubemap
        FragColor = texture(skybox, TexCoords);
    } 
    else if (skyboxType == 1) { // Equirectangular
        vec2 uv = sampleSphericalMap(normalize(TexCoords));
        FragColor = texture(equirectangularMap, uv);
    }
    else if (skyboxType == 2){ // Hemisphere
        float factor = clamp(TexCoords.y, 0.0, 1.0);
        FragColor = vec4(mix(horizonColor, skyColor, factor), 1.0);
        
        // 添加简单的太阳
        vec3 sunDir = normalize(vec3(0.5, 0.8, 0.6));
        float sun = max(dot(normalize(TexCoords), sunDir), 0.0);
        sun = pow(sun, 32.0);
        FragColor.rgb += vec3(sun * 0.8);
    }
    else if (skyboxType == 3){
                // 基础天空颜色
        float horizonMix = smoothstep(-0.1, 0.1, dir.y);
        vec3 bgColor = mix(vec3(0.0), skyColor, horizonMix);
        
        // 生成星空元素
        float stars = generateStars(TexCoords);
        float galaxy = generateGalaxy(TexCoords);
        //float galaxy = smoothstep(0.7, 0.9, 
        //gradientNoise(dir * 10.0 + vec3(u_time * 0.01))) * 
        //(1.0 - smoothstep(0.0, 0.5, abs(dir.y)));
        //vec3 nebula = generateNebula(TexCoords);
        vec3 nebula = renderVolumeNebula(u_cameraPos, dir);
        // 组合所有效果
        vec3 finalColor = bgColor;
        finalColor += stars * starColor;
        finalColor += galaxy * galaxyCoreColor;
        finalColor += nebula;
        
        FragColor = vec4(finalColor, 1.0);
        
        // 添加简单的太阳 (可选)
        vec3 sunDir = normalize(vec3(sin(u_time * 0.01), cos(u_time* 0.01), 0));
        float sun = max(dot(dir, sunDir), 0.0);
        sun = pow(sun, 256.0);
        FragColor.rgb += vec3(sun * 1.5);

    }
}