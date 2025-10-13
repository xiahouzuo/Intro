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
    // **调试1：直接输出光的颜色**
    if (vUV.x > 0.95 && vUV.y > 0.95) {
        if (lights.numDir > 0) {
            FragColor = vec4(lights.dirLights[0].color.rgb, 1.0);
        } else {
            FragColor = vec4(1.0, 0.0, 1.0, 1.0); // 紫色表示没有方向光
        }
        return;
    }
    
    // **调试2：检查是否访问了正确的UBO绑定点**
    vec3 debugColor = vec3(0.0);
    if (lights.numDir > 0) {
        debugColor = lights.dirLights[0].color.rgb;
    }
    
    // **调试3：强制测试不同的颜色组合**
    if (vUV.x < 0.05 && vUV.y > 0.95) {
        // 左上角：显示纯白色测试
        FragColor = vec4(1.0, 1.0, 1.0, 1.0);
        return;
    }
    
    if (vUV.x < 0.05 && vUV.y < 0.05) {
        // 左下角：显示硬编码的颜色
        FragColor = vec4(0.0, 1.0, 0.0, 1.0); // 绿色
        return;
    }
    
    // 正常的渲染逻辑
    vec3 diffuseMap = texture(material_diffuse, vUV).rgb;
    vec3 normal = normalize(vNormal);
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

    result = clamp(result, 0.0, 1.0);
    FragColor = vec4(result, 1.0);

}