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

out vec4 FragColor;

uniform sampler2D material_diffuse;
uniform sampler2D material_specular;
uniform float material_shininess;
uniform vec3 u_AmbientColor;

// 点光源衰减函数
float CalculateAttenuation(float distance, float range) {
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
    return clamp(attenuation, 0.0, 1.0) * smoothstep(range, range * 0.5, distance);
}

// 聚光灯强度计算
float CalculateSpotIntensity(vec3 lightDir, vec3 spotDir, float outerCos, float innerCos) {
    float theta = dot(lightDir, spotDir);
    float epsilon = innerCos - outerCos;
    return clamp((theta - outerCos) / epsilon, 0.0, 1.0);
}

void main() {

    
    // 正常的渲染逻辑
    vec3 diffuseMap = texture(material_diffuse, vUV).rgb;
    vec3 normal = normalize(vNormal);
    vec3 viewDir = normalize(camera.viewPos.xyz - vFragPos);
    vec3 result = vec3(0.0);

    // 环境光
    result += u_AmbientColor * diffuseMap;

    // 方向光
    for (int i = 0; i < lights.numDir && i < 4; i++) {
        vec3 lightDir = normalize(-lights.dirLights[i].direction.xyz);
        float diff = max(dot(normal, lightDir), 0.0);
        
        vec3 lightColor = lights.dirLights[i].color.rgb;
        vec3 diffuse = diff * diffuseMap * lightColor;
        result += diffuse;
    }

    // 点光源
    for (int i = 0; i < lights.numPoint && i < 8; i++) {
        vec3 lightPos = lights.pointLights[i].position.xyz;
        float lightRange = lights.pointLights[i].position.w;
        
        vec3 lightDir = normalize(lightPos - vFragPos);
        float diff = max(dot(normal, lightDir), 0.0);
        
        // 计算衰减
        float distance = length(lightPos - vFragPos);
        float attenuation = CalculateAttenuation(distance, lightRange);
        
        vec3 lightColor = lights.pointLights[i].color.rgb;
        vec3 diffuse = diff * diffuseMap * lightColor * attenuation;
        result += diffuse;
    }

    // 聚光灯
    for (int i = 0; i < lights.numSpot && i < 4; i++) {
        vec3 lightPos = lights.spotLights[i].position.xyz;
        float lightRange = lights.spotLights[i].position.w;
        vec3 spotDir = normalize(-lights.spotLights[i].direction.xyz);
        float outerCos = lights.spotLights[i].direction.w;
        float innerCos = lights.spotLights[i].params.x;
        
        vec3 lightDir = normalize(lightPos - vFragPos);
        float diff = max(dot(normal, lightDir), 0.0);
        
        // 计算衰减
        float distance = length(lightPos - vFragPos);
        float attenuation = CalculateAttenuation(distance, lightRange);
        
        // 计算聚光灯强度
        float spotIntensity = CalculateSpotIntensity(lightDir, spotDir, outerCos, innerCos);
        
        vec3 lightColor = lights.spotLights[i].color.rgb;
        vec3 diffuse = diff * diffuseMap * lightColor * attenuation * spotIntensity;
        result += diffuse;
    }

    result = clamp(result, 0.0, 1.0);
    FragColor = vec4(result, 1.0);
}