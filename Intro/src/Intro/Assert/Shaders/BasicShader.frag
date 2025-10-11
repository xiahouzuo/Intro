#version 420 core

// UBO����
layout(std140, binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;
    vec4 viewPos;
    float time;
    vec3 pad;
} camera;

// ��Դ�ṹ�嶨��
struct DirLight {
    vec4 direction;
    vec4 color;
};

struct PointLight {
    vec4 position;
    vec4 color;
};

struct SpotLight {
    vec4 position;
    vec4 direction;
    vec4 color;
    vec4 params;
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

// ����uniforms - �޸����Ƴ���ʼ����
uniform sampler2D material_diffuse;
uniform sampler2D material_specular;
uniform float material_shininess;
uniform vec3 u_AmbientColor;  // ��C++������

// ���ռ��㺯�� - �޸�������Ԥ��������
vec3 CalcDirectional(int index, vec3 normal, vec3 viewDir, vec3 diffuseMap, vec3 specularMap) {
    vec3 lightDir = normalize(-lights.dirLights[index].direction.xyz);
    float diff = max(dot(normal, lightDir), 0.0);
    
    vec3 diffuse = diff * diffuseMap * lights.dirLights[index].color.rgb * lights.dirLights[index].color.w;
    
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material_shininess);
    vec3 specular = spec * specularMap * lights.dirLights[index].color.rgb * lights.dirLights[index].color.w;
    
    return diffuse + specular;
}

vec3 CalcPoint(int index, vec3 normal, vec3 viewDir, vec3 diffuseMap, vec3 specularMap) {
    vec3 lightVec = lights.pointLights[index].position.xyz - vFragPos;
    float distance = length(lightVec);
    vec3 lightDir = lightVec / distance;
    
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * diffuseMap * lights.pointLights[index].color.rgb * lights.pointLights[index].color.w;
    
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material_shininess);
    vec3 specular = spec * specularMap * lights.pointLights[index].color.rgb * lights.pointLights[index].color.w;
    
    // ˥������
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));
    float range = lights.pointLights[index].position.w;
    if (distance > range) attenuation = 0.0;
    
    return (diffuse + specular) * attenuation;
}

vec3 CalcSpot(int index, vec3 normal, vec3 viewDir, vec3 diffuseMap, vec3 specularMap) {
    vec3 lightVec = lights.spotLights[index].position.xyz - vFragPos;
    float distance = length(lightVec);
    vec3 lightDir = lightVec / distance;
    
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * diffuseMap * lights.spotLights[index].color.rgb * lights.spotLights[index].color.w;
    
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material_shininess);
    vec3 specular = spec * specularMap * lights.spotLights[index].color.rgb * lights.spotLights[index].color.w;
    
    // �۹��ǿ�ȼ��� - �޸�����ֹ����
    float theta = dot(lightDir, normalize(-lights.spotLights[index].direction.xyz));
    float epsilon = lights.spotLights[index].direction.w - lights.spotLights[index].params.x;
    float intensity = clamp((theta - lights.spotLights[index].params.x) / max(epsilon, 0.001), 0.0, 1.0);
    
    // ����˥��
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));
    float range = lights.spotLights[index].position.w;
    if (distance > range) attenuation = 0.0;
    
    return (diffuse + specular) * attenuation * intensity;
}

void main() {
    // �޸�����ǰ�������������ظ�����
    vec3 diffuseMap = texture(material_diffuse, vUV).rgb;
    vec3 specularMap = texture(material_specular, vUV).rgb;

    vec3 normal = normalize(vNormal);
    vec3 viewDir = normalize(camera.viewPos.xyz - vFragPos);
    vec3 result = vec3(0.0);

    // ������
    vec3 ambient = u_AmbientColor * diffuseMap;
    result += ambient;

    // �����
    for (int i = 0; i < lights.numDir && i < 4; i++) {
        result += CalcDirectional(i, normal, viewDir, diffuseMap, specularMap);
    }
    
    // ���Դ
    for (int i = 0; i < lights.numPoint && i < 8; i++) {
        result += CalcPoint(i, normal, viewDir, diffuseMap, specularMap);
    }
    
    // �۹��
    for (int i = 0; i < lights.numSpot && i < 4; i++) {
        result += CalcSpot(i, normal, viewDir, diffuseMap, specularMap);
    }

    FragColor = vec4(result, 1.0);
}