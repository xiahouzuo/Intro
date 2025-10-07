#version 420 core

// 手动嵌入 camera.glsl 内容
layout(std140, binding = 0) uniform CameraUBO {
    mat4 u_View;
    mat4 u_Proj;
    vec4 u_ViewPos; // xyz pos, w unused
    float u_Time;
    vec3 _pad1;
};

// 手动嵌入 lights.glsl 内容
const int MAX_DIR_LIGHTS = 4;
const int MAX_POINT_LIGHTS = 8;
const int MAX_SPOT_LIGHTS = 4;

struct DirLight {
    vec4 direction; // xyz = dir, w unused
    vec4 color;     // rgb = color, w = intensity
};

struct PointLight {
    vec4 position;  // xyz = pos, w = range
    vec4 color;     // rgb = color, w = intensity
};

struct SpotLight {
    vec4 position;   // xyz = pos, w = range
    vec4 direction;  // xyz = dir, w = innerCos
    vec4 color;      // rgb = color, w = intensity
    vec4 params;     // x=outerCos, y=innerCos, z=range, w reserved
};

layout(std140, binding = 1) uniform LightsUBO {
    int numDir;
    int numPoint;
    int numSpot;
    vec2 _pad0;
    DirLight dirLights[MAX_DIR_LIGHTS];
    PointLight pointLights[MAX_POINT_LIGHTS];
    SpotLight spotLights[MAX_SPOT_LIGHTS];
};

in vec3 vFragPos;
in vec3 vNormal;
in vec2 vUV;

out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};
uniform Material material;

uniform vec3 u_AmbientColor = vec3(0.1f);

// 修正光照计算函数的参数列表（移除多余逗号，修正变量引用）
vec3 CalcDirectional(DirLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction.xyz);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color.rgb * light.color.w;
    
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = spec * light.color.rgb * light.color.w;
    
    return diffuse + specular;
}

vec3 CalcPoint(PointLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(light.position.xyz - vFragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color.rgb * light.color.w;
    
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = spec * light.color.rgb * light.color.w;
    
    // 衰减计算
    float distance = length(light.position.xyz - vFragPos);
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));
    if (distance > light.position.w) attenuation = 0.0; // 超出范围
    
    return (diffuse + specular) * attenuation;
}

vec3 CalcSpot(SpotLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(light.position.xyz - vFragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color.rgb * light.color.w;
    
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = spec * light.color.rgb * light.color.w;
    
    // 聚光范围计算
    float theta = dot(lightDir, normalize(-light.direction.xyz));
    float epsilon = light.direction.w - light.params.x; // innerCos - outerCos
    float intensity = clamp((theta - light.params.x) / epsilon, 0.0, 1.0);
    
    // 衰减计算
    float distance = length(light.position.xyz - vFragPos);
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));
    if (distance > light.position.w) attenuation = 0.0;
    
    return (diffuse + specular) * attenuation * intensity;
}

void main() {
    vec3 normal = normalize(vNormal);
    vec3 viewDir = normalize(u_ViewPos.xyz - vFragPos);
    vec3 result = vec3(0.0);

    // 累加所有光源贡献
    for (int i = 0; i < numDir; i++)
        result += CalcDirectional(dirLights[i], normal, viewDir);
    
    for (int i = 0; i < numPoint; i++)
        result += CalcPoint(pointLights[i], normal, viewDir);
    
    for (int i = 0; i < numSpot; i++)
        result += CalcSpot(spotLights[i], normal, viewDir);

    // 环境光
    vec3 ambient = u_AmbientColor * texture(material.diffuse, vUV).rgb;
    FragColor = vec4(ambient + result, 1.0);
}