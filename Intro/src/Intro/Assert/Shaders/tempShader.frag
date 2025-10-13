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

layout(std140, binding = 1) uniform LightsUBO {
    int numDir;
    int numPoint;
    int numSpot;
    vec2 pad;
    
    DirLight dirLights[4];
} lights;

in vec3 vFragPos;
in vec3 vNormal;
in vec2 vUV;

out vec4 FragColor;

uniform sampler2D material_diffuse;
uniform sampler2D material_specular;
uniform float material_shininess;
uniform vec3 u_AmbientColor;

void main() {
    // **����1��ֱ����������ɫ**
    if (vUV.x > 0.95 && vUV.y > 0.95) {
        if (lights.numDir > 0) {
            FragColor = vec4(lights.dirLights[0].color.rgb, 1.0);
        } else {
            FragColor = vec4(1.0, 0.0, 1.0, 1.0); // ��ɫ��ʾû�з����
        }
        return;
    }
    
    // **����2������Ƿ��������ȷ��UBO�󶨵�**
    vec3 debugColor = vec3(0.0);
    if (lights.numDir > 0) {
        debugColor = lights.dirLights[0].color.rgb;
    }
    
    // **����3��ǿ�Ʋ��Բ�ͬ����ɫ���**
    if (vUV.x < 0.05 && vUV.y > 0.95) {
        // ���Ͻǣ���ʾ����ɫ����
        FragColor = vec4(1.0, 1.0, 1.0, 1.0);
        return;
    }
    
    if (vUV.x < 0.05 && vUV.y < 0.05) {
        // ���½ǣ���ʾӲ�������ɫ
        FragColor = vec4(0.0, 1.0, 0.0, 1.0); // ��ɫ
        return;
    }
    
    // ��������Ⱦ�߼�
    vec3 diffuseMap = texture(material_diffuse, vUV).rgb;
    vec3 normal = normalize(vNormal);
    vec3 result = vec3(0.0);

    // ������
    result += u_AmbientColor * diffuseMap;

    // �����
    for (int i = 0; i < lights.numDir && i < 4; i++) {
        vec3 lightDir = normalize(-lights.dirLights[i].direction.xyz);
        float diff = max(dot(normal, lightDir), 0.0);
        
        vec3 lightColor = lights.dirLights[i].color.rgb;
        vec3 diffuse = diff * diffuseMap * lightColor;
        result += diffuse;
    }

    result = clamp(result, 0.0, 1.0);
    FragColor = vec4(result, 1.0);

}