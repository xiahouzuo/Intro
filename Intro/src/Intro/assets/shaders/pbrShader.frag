#version 420 core

layout(std140, binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;
    vec4 viewPos;
    float time;
    vec3 pad;
} camera;

struct DirLight {
    vec4 direction;
    vec4 color;
    vec4 intensity; // x=intensity, yzw=pad
};

struct PointLight {
    vec4 position;  // xyz=pos, w=range
    vec4 color;     // rgb=color, w=intensity
};

struct SpotLight {
    vec4 position;  // xyz=pos, w=range
    vec4 direction; // xyz=direction, w=outerCos
    vec4 color;     // rgb=color, w=intensity
    vec4 params;    // x=innerCos, y=unused, z=unused, w=unused
};

layout(std140, binding = 1) uniform LightsUBO {
    int numDir;
    int numPoint;
    int numSpot;
    vec2 pad;
    
    DirLight dirLights[4];
    PointLight pointLights[8];
    SpotLight spotLights[4];
} lights;

in vec3 vFragPos;
in vec3 vNormal;
in vec2 vUV;
in mat3 vTBN;

out vec4 FragColor;

// PBR材质贴图
uniform sampler2D material_albedo;
uniform sampler2D material_normal;
uniform sampler2D material_metallic;
uniform sampler2D material_roughness;
uniform sampler2D material_ao;
uniform sampler2D material_emissive;

// IBL贴图
uniform samplerCube u_IrradianceMap;
uniform samplerCube u_PrefilterMap;
uniform sampler2D u_BRDFLUT;

uniform vec3 u_AlbedoColor;
uniform float u_Metallic;
uniform float u_Roughness; 
uniform float u_AO;
uniform vec3 u_EmissiveColor;

// 纹理使用标志
uniform int u_UseAlbedoMap;
uniform int u_UseNormalMap;
uniform int u_UseMetallicMap;
uniform int u_UseRoughnessMap;
uniform int u_UseAOMap;
uniform int u_UseEmissiveMap;

uniform vec3 u_AmbientColor;
uniform float u_Exposure;

const float PI = 3.14159265359;

// 伽马校正
vec3 gammaCorrect(vec3 color) {
    return pow(color, vec3(1.0/2.2));
}

// 色调映射
vec3 toneMapping(vec3 color) {
    // ACES色调映射
    color *= u_Exposure;
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

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

// 菲涅尔方程 (Fresnel-Schlick)
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// 菲涅尔方程带粗糙度
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// 点光衰减
float CalculateAttenuation(float distance, float range) {
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
    return clamp(attenuation, 0.0, 1.0) * smoothstep(range, range * 0.5, distance);
}

// 聚光灯强度
float CalculateSpotIntensity(vec3 lightDir, vec3 spotDir, float outerCos, float innerCos) {
    float theta = dot(lightDir, spotDir);
    float epsilon = innerCos - outerCos;
    return clamp((theta - outerCos) / epsilon, 0.0, 1.0);
}

void main() {
    // 读取材质属性
   vec3 albedo = u_UseAlbedoMap == 1 ? 
        pow(texture(material_albedo, vUV).rgb, vec3(2.2)) : 
        u_AlbedoColor;
    
    float metallic = u_UseMetallicMap == 1 ? 
        texture(material_metallic, vUV).r : 
        u_Metallic;
        
    float roughness = u_UseRoughnessMap == 1 ? 
        texture(material_roughness, vUV).r : 
        u_Roughness;
        
    float ao = u_UseAOMap == 1 ? 
        texture(material_ao, vUV).r : 
        u_AO;
        
    vec3 emissive = u_UseEmissiveMap == 1 ? 
        texture(material_emissive, vUV).rgb : 
        u_EmissiveColor;
    
    
    // 法线贴图
    vec3 normal = texture(material_normal, vUV).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    normal = normalize(vTBN * normal);
    
    vec3 N = normal;
    vec3 V = normalize(camera.viewPos.xyz - vFragPos);
    vec3 R = reflect(-V, N);
    
    // 计算基础反射率
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    
    // 反射方程
    vec3 Lo = vec3(0.0);
    
    // 方向光
    for (int i = 0; i < lights.numDir && i < 4; i++) {
        vec3 L = normalize(-lights.dirLights[i].direction.xyz);
        vec3 H = normalize(V + L);
        float distance = 1.0;
        float attenuation = 1.0;
        vec3 radiance = lights.dirLights[i].color.rgb * lights.dirLights[i].intensity.x;
        
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
        Lo += (kD * albedo / PI + specular) * radiance * NdotL * attenuation;
    }
    
    // 点光源
    for (int i = 0; i < lights.numPoint && i < 8; i++) {
        vec3 lightPos = lights.pointLights[i].position.xyz;
        float lightRange = lights.pointLights[i].position.w;
        
        vec3 L = normalize(lightPos - vFragPos);
        vec3 H = normalize(V + L);
        float distance = length(lightPos - vFragPos);
        float attenuation = CalculateAttenuation(distance, lightRange);
        vec3 radiance = lights.pointLights[i].color.rgb * lights.pointLights[i].color.w * attenuation;
        
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
    
    // 聚光灯
    for (int i = 0; i < lights.numSpot && i < 4; i++) {
        vec3 lightPos = lights.spotLights[i].position.xyz;
        float lightRange = lights.spotLights[i].position.w;
        vec3 spotDir = normalize(-lights.spotLights[i].direction.xyz);
        float outerCos = lights.spotLights[i].direction.w;
        float innerCos = lights.spotLights[i].params.x;
        
        vec3 L = normalize(lightPos - vFragPos);
        vec3 H = normalize(V + L);
        float distance = length(lightPos - vFragPos);
        float attenuation = CalculateAttenuation(distance, lightRange);
        float spotIntensity = CalculateSpotIntensity(L, spotDir, outerCos, innerCos);
        vec3 radiance = lights.spotLights[i].color.rgb * lights.spotLights[i].color.w * attenuation * spotIntensity;
        
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
    
    // 环境光照 (IBL)
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;
    
    vec3 irradiance = texture(u_IrradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;
    
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(u_PrefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(u_BRDFLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
    
    vec3 ambient = (kD * diffuse + specular) * ao;
    
    // 最终颜色
    vec3 color = ambient + Lo + emissive;
    
    // 色调映射和伽马校正
    color = toneMapping(color);
    color = gammaCorrect(color);
    
    FragColor = vec4(color, 1.0);
}
