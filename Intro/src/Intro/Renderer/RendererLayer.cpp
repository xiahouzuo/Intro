#include "itrpch.h"
#include "RendererLayer.h"
#include "imgui.h"
#include "Intro/Application.h"
#include "Intro/ECS/SceneManager.h"
#include "RenderCommand.h"
#include <glad/glad.h>



#include <GLFW/glfw3.h>
namespace Intro {

	RendererLayer::RendererLayer(const Window& window)
		:Layer("Renderer Layer"), m_Window(window), m_Camera(window)
	{
		
		m_Shader = std::make_unique<Shader>("E:/MyEngine/Intro/Intro/src/Intro/Assert/Shaders/BasicShader.vert",
			"E:/MyEngine/Intro/Intro/src/Intro/Assert/Shaders/BasicShader.frag");
		
	}

	RendererLayer::~RendererLayer()
	{
		m_Shader.reset();
	}


	//void RendererLayer::InitShapes()
	//{
	//	std::vector<std::shared_ptr<Texture>> emptyTexture = {};

	//	auto [cubeVertices, cubeIndices] = ShapeGenerator::GenerateCube(1.0f);
	//	m_Shapes.push_back(std::make_unique<Mesh>(cubeVertices, cubeIndices, emptyTexture));

	//	auto [sphereVertices, sphereIndices] = ShapeGenerator::GenerateSphere(0.8f);
	//	m_Shapes.push_back(std::make_unique<Mesh>(sphereVertices, sphereIndices, emptyTexture));

	//	auto [planeVertices, planeIndices] = ShapeGenerator::GeneratePlane(2.0f, 2.0f);
	//	m_Shapes.push_back(std::make_unique<Mesh>(planeVertices, planeIndices, emptyTexture));
	//}

	void RendererLayer::OnAttach()
	{
		glEnable(GL_DEPTH_TEST);
	}

	void RendererLayer::OnUpdate(float deltaTime)
	{

		m_Camera.OnUpdate(deltaTime);

		auto& sceneMgr = Application::GetSceneManager();
		auto* activeScene = sceneMgr.GetActiveScene();
		if (!activeScene)
		{
			ITR_ERROR("No active Scene");
			return;
		}
		auto& ecs = activeScene->GetECS();

		m_RenderableData.clear();
		m_RenderableData = RenderSystem::GetRenderables(ecs);

		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_Shader->Bind();

		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = m_Camera.GetViewMat();
		glm::mat4 proj = m_Camera.GetProjectionMat();

		m_Shader->SetUniformMat4("view", view);
		m_Shader->SetUniformMat4("projection", proj);

		for (const auto& r : m_RenderableData) {
			// safety
			if (!r.mesh) continue;

			glm::mat4 model = r.transform.GetModelMatrix();
			m_Shader->SetUniformMat4("model", model);

			// 你的 DrawMesh 接口（负责绑定 VAO/材质并发 draw call）
			RenderCommand::Draw(*r.mesh, *m_Shader);
		}

		/*switch (m_CurrentShape) {
			case ShapeType::Null:
				glClear(GL_COLOR_BUFFER_BIT);
				break;
			case ShapeType::Cube:
				m_Shapes[0]->Draw(*m_Shader);
				break;
			case ShapeType::Sphere:
				m_Shapes[1]->Draw(*m_Shader);
				break;
			case ShapeType::Plane:
				m_Shapes[2]->Draw(*m_Shader);
				break;
		}
		m_Model->Draw(*m_Shader);*/
		m_Shader->UnBind();

	}

	void RendererLayer::OnEvent(Event& event)
	{
		EventDispatcher dispathcer(event);

		dispathcer.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(RendererLayer::OnWindowResized));
	}

	bool RendererLayer::OnWindowResized(WindowResizeEvent& e)
	{
		m_Camera.AspectRatio = ((float)e.GetWidth() / (float)e.GetHeight());
		return false;
	}


	
}