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
    layout (location = 0) in vec3 aPos;      // ��Mesh��Vertex.Position��Ӧ��location=0��
    layout (location = 1) in vec3 aNormal;   // ��ʱ���ã�����
    layout (location = 2) in vec2 aTexCoords;// ��ʱ���ã�����

    uniform mat4 model;      // ģ�;�����������ı任��
    uniform mat4 view;       // ��ͼ����������ı任��
    uniform mat4 projection; // ͶӰ����͸��/������

    void main() {
        // ���ģ��������� = ͶӰ���� * ��ͼ���� * ģ�;��� * �ֲ�����
        gl_Position = projection * view * model * vec4(aPos, 1.0);
    }
)";

        // Ƭ����ɫ��������̶���ɫ����ɫ����������ã�
        const std::string DefaultFragmentShader = R"(
    #version 330 core
    out vec4 FragColor; // �����ɫ

    void main() {
        FragColor = vec4(1.0, 0.0, 0.0, 1.0); // ��ɫ
    }
)";
	};
}