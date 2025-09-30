// ImGuiLayer.cpp
#include "itrpch.h"
#include "ImGuiLayer.h"
#include "Platform/OpenGL/ImGuiOpenGLRenderer.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_glfw.h"
#include "misc/cpp/imgui_stdlib.h"
#include "Intro/Application.h"
#include "Intro/ECS/Components.h"
#include "entt/entt.hpp"
#include <unordered_set>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Intro {

	ImGuiLayer::ImGuiLayer(SceneManager* sceneManager, RendererLayer* rendererLayer)
		: Layer("ImGuiLayer"), m_SceneManager(sceneManager), m_RendererLayer(rendererLayer),
		m_TransformEditor(glm::vec3(0.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.0f)),
		m_EulerAngles(0.0f)
	{
	}

	ImGuiLayer::~ImGuiLayer()
	{
		ImGui::DestroyContext();
	}

	/**
	 * 初始化ImGui上下文和后端
	 * 注意：需在OpenGL上下文创建后调用
	 */
	void ImGuiLayer::OnAttach()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		// 配置ImGui功能：启用键盘导航、停靠、多视口
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		// 设置样式为深色主题
		ImGui::StyleColorsDark();

		// 适配多视口样式
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		// 初始化GLFW和OpenGL后端
		GLFWwindow* window = (GLFWwindow*)Application::Get().GetWindow().GetNativeWindow();
		IM_ASSERT(window != nullptr);
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");

		// 初始刷新实体列表
		RefreshEntityList();
	}

	/**
	 * 清理ImGui资源
	 */
	void ImGuiLayer::OnDetach()
	{
		// 关闭后端并销毁上下文
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	/**
	 * 每帧更新UI逻辑
	 * 流程：开始帧 -> 绘制UI -> 渲染UI -> 处理多视口
	 */
	void ImGuiLayer::OnUpdate(float deltaTime)
	{
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		// 更新显示尺寸
		io.DisplaySize = ImVec2(app.GetWindow().GetWidth(), app.GetWindow().GetHeight());

		// 更新时间增量
		float time = (float)glfwGetTime();
		io.DeltaTime = m_Time > 0.0f ? (time - m_Time) : (1.0f / 60.0f);
		m_Time = time;

		// 开始新帧
		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();

		// 绘制UI组件（按顺序绘制，确保布局正确）
		DrawMenuBar();          // 菜单栏
		DrawDockSpaceHost();    // 停靠容器（基础布局）
		DrawViewport();         // 渲染视口
		ShowEntityManagerWindow(); // 实体管理器
		ShowSceneControlsWindow(); // 场景控制

		// 若有选中实体，同步数据并显示检查器
		if (m_SelectedEntity != entt::null)
		{
			SyncTransformEditor();
			ShowEntityInspectorWindow();
		}

		// 显示模型导入窗口（若开启）
		if (m_ShowImportWindow)
		{
			ShowImportModelWindow();
		}

		// 渲染UI
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// 处理多视口（保持与平台窗口同步）
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}

	/**
	 * 创建全局停靠容器
	 * 作用：作为所有UI窗口的根容器，支持窗口拖拽停靠
	 */
	void ImGuiLayer::DrawDockSpaceHost()
	{
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
		ImGuiViewport* viewport = ImGui::GetMainViewport();

		// 获取菜单栏高度：利用“先绘制菜单栏”的前提，取当前窗口（菜单栏）的高度
// 手动指定菜单栏高度（比如20像素，可根据实际情况调整）
		float menuBarHeight = 20.0f;

		ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + menuBarHeight));
		ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, viewport->Size.y - menuBarHeight));

		// 1. 调整DockSpace容器位置：Y轴偏移菜单栏高度
		ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + menuBarHeight));

		// 2. 调整DockSpace容器尺寸：高度 = 视口高度 - 菜单栏高度
		ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, viewport->Size.y - menuBarHeight));

		// 保持视口关联、设置窗口标志等（原代码不变）...
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGuiWindowFlags host_window_flags = ImGuiWindowFlags_NoTitleBar; /* 其他标志 */

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

		ImGui::Begin("DockSpaceHost", nullptr, host_window_flags);
		ImGui::PopStyleVar(3);

		ImGuiID dockspace_id = ImGui::GetID("EditorDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

		ImGui::End();
	}

	/**
	 * 绘制菜单栏
	 * 包含：文件操作（导入模型）、创建实体（几何体）等入口
	 */
	void ImGuiLayer::DrawMenuBar()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Import Model..."))
				{
					m_ShowImportWindow = true; // 打开导入窗口
				}
				ImGui::Separator();
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Create"))
			{
				if (ImGui::MenuItem("Cube")) CreatePrimitive(ShapeType::Cube);
				if (ImGui::MenuItem("Sphere")) CreatePrimitive(ShapeType::Sphere);
				if (ImGui::MenuItem("Plane")) CreatePrimitive(ShapeType::Plane);
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

	/**
	 * 绘制渲染视口
	 * 功能：显示场景渲染结果（从RendererLayer获取纹理）、检测尺寸变化、处理交互状态
	 */
	void ImGuiLayer::DrawViewport()
	{
		ImGui::Begin("Viewport");

		// 更新视口交互状态
		m_ViewportHovered = ImGui::IsWindowHovered();
		m_ViewportFocused = ImGui::IsWindowFocused();

		// 获取视口可用区域尺寸
		ImVec2 avail = ImGui::GetContentRegionAvail();
		m_ViewportSize = avail;
		m_ViewportOffset = ImGui::GetWindowPos();

		// 防止尺寸为0导致的渲染错误
		if (avail.x < 1.0f) avail.x = 1.0f;
		if (avail.y < 1.0f) avail.y = 1.0f;

		// 若视口尺寸变化，通知渲染层调整FBO
		if (m_RendererLayer &&
			((uint32_t)avail.x != m_LastViewportSize.x ||
				(uint32_t)avail.y != m_LastViewportSize.y))
		{
			m_RendererLayer->ResizeViewport((uint32_t)avail.x, (uint32_t)avail.y);
			m_LastViewportSize = avail;
		}

		// 获取渲染纹理并显示
		void* texID = nullptr;
		if (m_RendererLayer)
		{
			GLuint id = m_RendererLayer->GetSceneTextureID();
			texID = (void*)(intptr_t)id;
		}

		if (texID)
		{
			// 显示纹理（注意OpenGL纹理Y轴翻转，UV需调整）
			ImGui::Image(texID, avail, ImVec2(0, 1), ImVec2(1, 0));
		}
		else
		{
			ImGui::Text("No render target available");
			ImGui::Dummy(avail); // 占位符
		}

		// 实体拖拽目标（支持将实体拖入视口）
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_PAYLOAD"))
			{
				IM_ASSERT(payload->DataSize == sizeof(uint32_t));
				uint32_t entId = *(const uint32_t*)payload->Data;
				// 可在此处添加实体拖拽到视口的逻辑（如放置实体）
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::End();
	}

	/**
	 * 显示实体管理器窗口
	 * 功能：列出场景中所有实体、支持选择和拖拽
	 */
	void ImGuiLayer::ShowEntityManagerWindow()
	{
		ImGui::Begin("Entity Manager");

		// 刷新实体列表按钮
		if (ImGui::Button("Refresh List"))
		{
			RefreshEntityList();
		}

		ImGui::Separator();

		// 获取当前活动场景
		auto* activeScene = m_SceneManager ? m_SceneManager->GetActiveScene() : nullptr;
		if (!activeScene)
		{
			ImGui::Text("No active scene");
			ImGui::End();
			return;
		}

		// 显示实体列表
		auto& registry = activeScene->GetECS().GetRegistry();
		for (auto entity : m_CachedEntities)
		{
			if (!registry.valid(entity)) // 过滤无效实体
				continue;

			// 生成实体名称（优先显示标签，否则显示ID）
			std::string name = "Entity " + std::to_string((uint32_t)entity);
			if (registry.any_of<TagComponent>(entity))
			{
				name = registry.get<TagComponent>(entity).Tag;
			}

			// 显示实体项（支持选择）
			ImGui::PushID((uint32_t)entity); // 唯一ID，避免UI冲突
			if (ImGui::Selectable(name.c_str(), m_SelectedEntity == entity))
			{
				m_SelectedEntity = entity;
				m_SelectedEntityName = name;
			}

			// 支持拖拽实体
			if (ImGui::BeginDragDropSource())
			{
				uint32_t entId = (uint32_t)entity;
				ImGui::SetDragDropPayload("ENTITY_PAYLOAD", &entId, sizeof(uint32_t));
				ImGui::Text(name.c_str());
				ImGui::EndDragDropSource();
			}

			ImGui::PopID();
		}

		ImGui::End();
	}

	/**
	 * 显示实体检查器窗口
	 * 功能：编辑选中实体的组件（目前支持Transform）
	 */
	void ImGuiLayer::ShowEntityInspectorWindow()
	{
		ImGui::Begin("Inspector");

		if (m_SelectedEntity == entt::null)
		{
			ImGui::Text("No entity selected");
			ImGui::End();
			return;
		}

		// 获取当前场景和实体注册表
		auto* activeScene = m_SceneManager ? m_SceneManager->GetActiveScene() : nullptr;
		if (!activeScene)
		{
			ImGui::End();
			return;
		}

		auto& registry = activeScene->GetECS().GetRegistry();
		if (!registry.valid(m_SelectedEntity)) // 检查实体有效性
		{
			m_SelectedEntity = entt::null;
			ImGui::End();
			return;
		}

		// 显示实体名称
		ImGui::Text("Entity: %s", m_SelectedEntityName.c_str());
		ImGui::Separator();

		// 编辑Transform组件
		if (registry.any_of<TransformComponent>(m_SelectedEntity))
		{
			if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
			{
				// 显示位置、旋转（欧拉角）、缩放编辑框
				ImGui::DragFloat3("Position", &m_TransformEditor.position.x, 0.1f);
				ImGui::DragFloat3("Rotation", &m_EulerAngles.x, 1.0f); // 角度单位
				ImGui::DragFloat3("Scale", &m_TransformEditor.scale.x, 0.1f);

				// 应用变换到实体
				if (ImGui::Button("Apply Transform"))
				{
					// 欧拉角转四元数（内部存储用四元数避免万向锁）
					m_TransformEditor.rotation = glm::quat(glm::radians(m_EulerAngles));
					UpdateSelectedEntityTransform();
				}
			}
		}

		// 可扩展：添加其他组件（如Mesh、Light）的编辑逻辑

		ImGui::End();
	}

	/**
	 * 显示场景控制窗口
	 * 功能：场景级别的控制（如清空场景、保存场景等）
	 */
	void ImGuiLayer::ShowSceneControlsWindow()
	{
		ImGui::Begin("Scene Controls");

		// 示例：清空场景按钮
		if (ImGui::Button("Clear Scene") && m_SceneManager)
		{
			// m_SceneManager->ClearActiveScene(); // 实际项目中实现清空逻辑
		}

		ImGui::End();
	}

	/**
	 * 显示模型导入窗口
	 * 功能：输入模型路径并导入
	 */
	void ImGuiLayer::ShowImportModelWindow()
	{
		ImGui::Begin("Import Model", &m_ShowImportWindow);

		ImGui::InputText("Model Path", &m_ModelImportPath);
		ImGui::SameLine();
		if (ImGui::Button("Browse"))
		{
			// 实际项目中可调用文件选择对话框（如使用nfd库）
			// m_ModelImportPath = FileDialogs::OpenFile("Model Files (*.obj *.fbx *.gltf)");
		}

		if (ImGui::Button("Import"))
		{
			if (!m_ModelImportPath.empty())
			{
				ImportModel(m_ModelImportPath);
				m_ShowImportWindow = false;
			}
		}

		ImGui::End();
	}

	/**
	 * 刷新实体列表（从场景同步）
	 * 逻辑：收集所有包含TransformComponent的实体（确保是可渲染/可交互的实体）
	 */
	void ImGuiLayer::RefreshEntityList()
	{
		m_CachedEntities.clear();
		auto* activeScene = m_SceneManager ? m_SceneManager->GetActiveScene() : nullptr;
		if (!activeScene)
			return;

		auto& registry = activeScene->GetECS().GetRegistry();
		std::unordered_set<entt::entity> entities; // 用set去重

		// 收集所有有TransformComponent的实体（基础组件）
		auto view = registry.view<TransformComponent>();
		for (auto entity : view)
		{
			entities.insert(entity);
		}

		// 转换为vector便于遍历
		m_CachedEntities.assign(entities.begin(), entities.end());
	}

	/**
	 * 导入模型并创建实体
	 * @param modelPath 模型文件路径
	 * @return 是否导入成功
	 */
	bool ImGuiLayer::ImportModel(const std::string& modelPath)
	{
		if (!m_SceneManager) return false;

		auto* activeScene = m_SceneManager->GetActiveScene();
		if (!activeScene) return false;

		// 实际项目中应使用资源管理器加载模型
		// 示例逻辑：
		// auto model = ResourceManager::LoadModel(modelPath);
		// if (model)
		// {
		//     entt::entity ent = activeScene->CreateEntity("Model Entity");
		//     activeScene->GetECS().emplace<TransformComponent>(ent);
		//     activeScene->GetECS().emplace<ModelComponent>(ent, model);
		//     RefreshEntityList();
		//     return true;
		// }

		return false;
	}

	/**
	 * 创建基础几何体实体
	 * @param type 几何体类型（立方体、球体等）
	 */
	void ImGuiLayer::CreatePrimitive(ShapeType type)
	{
		if (!m_SceneManager) return;

		auto* activeScene = m_SceneManager->GetActiveScene();
		if (!activeScene) return;

		// 实际项目中使用ShapeGenerator创建网格
		// 示例逻辑：
		// std::string name;
		// std::shared_ptr<Mesh> mesh;
		// switch(type)
		// {
		//     case ShapeType::Cube:
		//         name = "Cube";
		//         mesh = ShapeGenerator::CreateCube();
		//         break;
		//     // 其他类型...
		// }
		// if (mesh)
		// {
		//     entt::entity ent = activeScene->CreateEntity(name);
		//     activeScene->GetECS().emplace<TransformComponent>(ent);
		//     activeScene->GetECS().emplace<MeshComponent>(ent, mesh);
		//     RefreshEntityList();
		// }
	}

	/**
	 * 将编辑器中的变换应用到选中实体
	 */
	void ImGuiLayer::UpdateSelectedEntityTransform()
	{
		auto* activeScene = m_SceneManager ? m_SceneManager->GetActiveScene() : nullptr;
		if (!activeScene || m_SelectedEntity == entt::null)
			return;

		auto& registry = activeScene->GetECS().GetRegistry();
		if (registry.any_of<TransformComponent>(m_SelectedEntity))
		{
			auto& transform = registry.get<TransformComponent>(m_SelectedEntity);
			transform.transform.position = m_TransformEditor.position;
			transform.transform.rotation = m_TransformEditor.rotation;
			transform.transform.scale = m_TransformEditor.scale;
		}
	}

	/**
	 * 同步选中实体的变换到编辑器（确保编辑的是最新数据）
	 */
	void ImGuiLayer::SyncTransformEditor()
	{
		auto* activeScene = m_SceneManager ? m_SceneManager->GetActiveScene() : nullptr;
		if (!activeScene || m_SelectedEntity == entt::null)
			return;

		auto& registry = activeScene->GetECS().GetRegistry();
		if (registry.any_of<TransformComponent>(m_SelectedEntity))
		{
			auto& transform = registry.get<TransformComponent>(m_SelectedEntity);
			// 同步位置、旋转、缩放
			m_TransformEditor.position = transform.transform.position;
			m_TransformEditor.rotation = transform.transform.rotation;
			m_TransformEditor.scale = transform.transform.scale;
			// 四元数转欧拉角（显示用）
			m_EulerAngles = glm::degrees(glm::eulerAngles(transform.transform.rotation));
		}
	}

	// 事件处理函数（保持原有事件系统逻辑，不修改返回值）
	bool ImGuiLayer::OnMouseButtonPressedEvent(MouseButtonPressedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MouseDown[e.GetMouseButton()] = true;
		return false; // 不拦截事件，按原有系统逻辑传播
	}

	bool ImGuiLayer::OnMouseButtonReleasedEvent(MouseButtonReleasedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MouseDown[e.GetMouseButton()] = false;
		return false;
	}

	bool ImGuiLayer::OnMouseMovedEvent(MouseMovedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MousePos = ImVec2(e.GetX(), e.GetY());
		return false;
	}

	bool ImGuiLayer::OnMouseScrolledEvent(MouseScrolledEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MouseWheelH += e.GetXOffset();
		io.MouseWheel += e.GetYOffset();
		return false;
	}

	bool ImGuiLayer::OnKeyPressedEvent(KeyPressedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeysDown[e.GetKeyCode()] = true;

		// 更新修饰键状态
		io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
		io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
		io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
		io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];

		return false;
	}

	bool ImGuiLayer::OnKeyReleasedEvent(KeyReleasedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeysDown[e.GetKeyCode()] = false;
		return false;
	}

	bool ImGuiLayer::OnKeyTypedEvent(KeyTypedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddInputCharacter(e.GetKeyCode());
		return false;
	}

	bool ImGuiLayer::OnWindowResizedEvent(WindowResizeEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(e.GetWidth(), e.GetHeight());
		io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
		return false;
	}

	/**
	 * 事件分发（绑定事件处理函数，不修改原有事件系统）
	 */
	void ImGuiLayer::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		// 绑定事件处理函数（使用原有系统的BIND_EVENT_FN宏）
		dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FN(ImGuiLayer::OnMouseButtonPressedEvent));
		dispatcher.Dispatch<MouseButtonReleasedEvent>(BIND_EVENT_FN(ImGuiLayer::OnMouseButtonReleasedEvent));
		dispatcher.Dispatch<MouseMovedEvent>(BIND_EVENT_FN(ImGuiLayer::OnMouseMovedEvent));
		dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FN(ImGuiLayer::OnMouseScrolledEvent));
		dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(ImGuiLayer::OnKeyPressedEvent));
		dispatcher.Dispatch<KeyReleasedEvent>(BIND_EVENT_FN(ImGuiLayer::OnKeyReleasedEvent));
		dispatcher.Dispatch<KeyTypedEvent>(BIND_EVENT_FN(ImGuiLayer::OnKeyTypedEvent));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(ImGuiLayer::OnWindowResizedEvent));
	}

}