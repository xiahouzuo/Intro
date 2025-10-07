#version 420 core

// �ֶ�Ƕ�� camera.glsl ���ݣ���� #include��
layout(std140, binding = 0) uniform CameraUBO {
    mat4 u_View;
    mat4 u_Proj;
    vec4 u_ViewPos; // xyz pos, w unused
    float u_Time;
    vec3 _pad1;
};

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

out vec3 vFragPos;
out vec3 vNormal;
out vec2 vUV;

uniform mat4 u_Transform; // object model

void main() {
    vec4 worldPos = u_Transform * vec4(aPos, 1.0);
    vFragPos = worldPos.xyz;
    mat3 normalMat = transpose(inverse(mat3(u_Transform)));
    vNormal = normalize(normalMat * aNormal);
    vUV = aUV;
    gl_Position = u_Proj * u_View * worldPos; // ʹ�� CameraUBO �е� u_Proj �� u_View
}