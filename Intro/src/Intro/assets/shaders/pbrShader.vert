#version 420 core

layout(std140, binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;
    vec4 viewPos;
    float time;
    vec3 pad;
} camera;

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

out vec3 vFragPos;
out vec3 vNormal;
out vec2 vUV;
out mat3 vTBN;

uniform mat4 u_Transform;

void main() {
    vec4 worldPos = u_Transform * vec4(aPos, 1.0);
    vFragPos = worldPos.xyz;
    
    // 法线矩阵变换
    mat3 normalMat = transpose(inverse(mat3(u_Transform)));
    vNormal = normalize(normalMat * aNormal);
    
    // 计算TBN矩阵用于法线贴图
    vec3 T = normalize(normalMat * aTangent);
    vec3 B = normalize(normalMat * aBitangent);
    vec3 N = normalize(normalMat * aNormal);
    vTBN = mat3(T, B, N);
    
    vUV = aUV;
    gl_Position = camera.proj * camera.view * worldPos;
}