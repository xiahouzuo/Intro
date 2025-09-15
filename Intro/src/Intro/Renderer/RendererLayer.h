#pragma once
#include "Intro/Core.h"
#include "Intro/Layer.h"
#include "Intro/Window.h"
#include "Camera.h"
#include "Shader.h"
#include "Mesh.h"
#include <memory>

namespace Intro {

	class ITR_API RendererLayer : public Layer
	{
	public:
		RendererLayer(const Window& window);

		void OnUpdate(float deltaTime);
	private:
		void InitTestMesh();

	private:
		const Window& m_Window;
		Camera m_Camera;
		std::unique_ptr<Shader> m_Shader;

		std::unique_ptr<Mesh> m_TestMesh;

        const std::string DefaultVertexShader = R"(
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
)";

        // 片段着色器：输出固定颜色（红色，当天测试用）
        const std::string DefaultFragmentShader = R"(
    #version 330 core
    out vec4 FragColor; // 输出颜色

    void main() {
        FragColor = vec4(1.0, 0.0, 0.0, 1.0); // 红色
    }
)";
	};
}