#include "itrpch.h"
#include "RendererLayer.h"
#include <glad/glad.h>


namespace Intro {

	RendererLayer::RendererLayer(const Window& window)
		:Layer("Renderer Layer"), m_Window(window), m_Camera(window)
	{
		InitTestMesh();
		m_Shader = std::make_unique<Shader>(DefaultVertexShader, DefaultFragmentShader);
	}

	void RendererLayer::OnUpdate(float deltaTime)
	{
		glEnable(GL_DEPTH_TEST);

		m_Camera.OnUpdate(deltaTime);
		glClearColor(0.9f, 0.5f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_Shader->Bind();

		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = m_Camera.GetViewMat();
		glm::mat4 proj = m_Camera.GetProjectionMat();

		m_Shader->SetUniformMat4("model", model);
		m_Shader->SetUniformMat4("view", view);
		m_Shader->SetUniformMat4("projection", proj);

		m_TestMesh->Draw();

		m_Shader->UnBind();

	}

	void RendererLayer::InitTestMesh()
	{
		std::vector<Vertex> vertices = {
			// 顶点1：位置(-1.0,-0.5,0.0)，法线(0,0,1)，UV(0,0)
			Vertex(glm::vec3(-1.0f, -0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)),
			// 顶点2：位置(0.0,1.0,0.0)，法线(0,0,1)，UV(0.5,1.0)
			Vertex(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.5f, 1.0f)),
			// 顶点3：位置(1.0,-0.5,0.0)，法线(0,0,1)，UV(1.0,0.0)
			Vertex(glm::vec3(1.0f, -0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f))
		};
		std::vector<unsigned int> indices = { 0, 1, 2 }; // 三角形的索引（按顺序绘制3个顶点）
		m_TestMesh = std::make_unique<Mesh>(vertices, indices);
	}
}