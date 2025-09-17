#version 330 core

layout (location = 0) in vec3 aPos;      // 与Mesh的Vertex.Position对应（location=0）
layout (location = 1) in vec3 aNormal;   // 暂时不用，保留
layout (location = 2) in vec2 aTexCoords;// 暂时不用，保留

uniform mat4 model;      // 模型矩阵（物体自身的变换）
uniform mat4 view;       // 视图矩阵（摄像机的变换）
uniform mat4 projection; // 投影矩阵（透视/正交）

void main() {
    // 核心：顶点坐标 = 投影矩阵 * 视图矩阵 * 模型矩阵 * 局部坐标
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}