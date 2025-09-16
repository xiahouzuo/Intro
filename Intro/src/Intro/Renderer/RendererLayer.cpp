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


		GLFWwindow* window = (GLFWwindow*)m_Window.GetNativeWindow();  // 请确保你的 Window 类实现了这个方法
		IM_ASSERT(window != nullptr);  // 确保窗口指针有效
		ImGui_ImplGlfw_InitForOpenGL(window, true);  // 绑定 GLFW 窗口

		// 3. 初始化 OpenGL 后端（已存在，但版本号需匹配你的环境）
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
		ImGui_ImplOpenGL3_NewFrame();  // OpenGL 后端帧开始
		ImGui_ImplGlfw_NewFrame();     // GLFW 后端帧开始
		ImGui::NewFrame();

		ImGui::Begin("Shape Generator");

		const char* shapeNames[] = { "Cube", "Sphere", "Plane" };
		if (ImGui::Combo("Shape type", (int*)&m_CurrentShape, shapeNames, IM_ARRAYSIZE(shapeNames)))
		{
			UpdateCurrentShape();
		}


		ImGui::End();

		ImGui::Render();  // 生成绘制数据
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	}


	void RendererLayer::UpdateCurrentShape()
	{

	}

	
}