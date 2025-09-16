#include "itrpch.h"
#include "RendererLayer.h"
#include "imgui.h"
#include <glad/glad.h>
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
namespace Intro {

	RendererLayer::RendererLayer(const Window& window)
		:Layer("Renderer Layer"), m_Window(window), m_Camera(window)
	{
		
		m_Shader = std::make_unique<Shader>(DefaultVertexShader, DefaultFragmentShader);
		InitShapes();
	}

	void RendererLayer::InitShapes()
	{
		auto [cubeVertices, cubeIndices] = ShapeGenerator::GenerateCube(1.0f);
		m_Shapes.push_back(std::make_unique<Mesh>(cubeVertices, cubeIndices));

		auto [sphereVertices, sphereIndices] = ShapeGenerator::GenerateSphere(0.8f);
		m_Shapes.push_back(std::make_unique<Mesh>(sphereVertices, sphereIndices));

		auto [planeVertices, planeIndices] = ShapeGenerator::GeneratePlane(2.0f, 2.0f);
		m_Shapes.push_back(std::make_unique<Mesh>(planeVertices, planeIndices));
	}

	void RendererLayer::OnAttach()
	{
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGuiIO& io = ImGui::GetIO();
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;


		GLFWwindow* window = (GLFWwindow*)m_Window.GetNativeWindow();  // ��ȷ����� Window ��ʵ�����������
		IM_ASSERT(window != nullptr);  // ȷ������ָ����Ч
		ImGui_ImplGlfw_InitForOpenGL(window, true);  // �� GLFW ����

		// 3. ��ʼ�� OpenGL ��ˣ��Ѵ��ڣ����汾����ƥ����Ļ�����
		ImGui_ImplOpenGL3_Init("#version 410");
	}

	void RendererLayer::OnUpdate(float deltaTime)
	{
		glEnable(GL_DEPTH_TEST);

		m_Camera.OnUpdate(deltaTime);
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_Shader->Bind();

		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = m_Camera.GetViewMat();
		glm::mat4 proj = m_Camera.GetProjectionMat();

		m_Shader->SetUniformMat4("model", model);
		m_Shader->SetUniformMat4("view", view);
		m_Shader->SetUniformMat4("projection", proj);

		switch (m_CurrentShape) {
			case ShapeType::Cube:
				m_Shapes[0]->Draw();
				break;
			case ShapeType::Sphere:
				m_Shapes[1]->Draw();
				break;
			case ShapeType::Plane:
				m_Shapes[2]->Draw();
				break;
		}

		m_Shader->UnBind();

	}

	void RendererLayer::OnImGuiRender()
	{
		ImGui_ImplOpenGL3_NewFrame();  // OpenGL ���֡��ʼ
		ImGui_ImplGlfw_NewFrame();     // GLFW ���֡��ʼ
		ImGui::NewFrame();

		ImGui::Begin("Shape Generator");

		const char* shapeNames[] = { "Cube", "Sphere", "Plane" };
		if (ImGui::Combo("Shape type", (int*)&m_CurrentShape, shapeNames, IM_ARRAYSIZE(shapeNames)))
		{
			UpdateCurrentShape();
		}


		ImGui::End();

		ImGui::Render();  // ���ɻ�������
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	}


	void RendererLayer::UpdateCurrentShape()
	{

	}

	
}