// ImGuiLayer.cpp
#include "itrpch.h"
#include "Intro/Application.h"
#include "Intro/ECS/Components.h"
#include "Intro/Renderer/ShapeGenerator.h"
#include "ImGuiLayer.h"
#include "Platform/OpenGL/ImGuiOpenGLRenderer.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_glfw.h"
#include "misc/cpp/imgui_stdlib.h"
#include "entt/entt.hpp"
#include <unordered_set>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Intro {

	ImGuiLayer::ImGuiLayer(SceneManager* sceneManager, RendererLayer* rendererLayer)
		: Layer("ImGuiLayer"), m_SceneManager(sceneManager), m_RendererLayer(rendererLayer),
		m_TransformEditor(glm::vec3(0.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.0f)),
		m_EulerAngles(0.0f)
	{


		defaultShader = std::make_shared<Shader>(
			"E:/MyEngine/Intro/Intro/src/Intro/Assert/Shaders/BasicShader.vert",
			"E:/MyEngine/Intro/Intro/src/Intro/Assert/Shaders/BasicShader.frag"
		);
		defaultMaterial = std::make_shared<Material>(defaultShader);

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

		ImGuizmo::SetImGuiContext(ImGui::GetCurrentContext());

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


		// 每帧初始化 ImGuizmo
		ImGuizmo::BeginFrame();

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
			RenderGizmo();
		}

		// 显示模型导入窗口（若开启）
		if (m_ShowImportWindow)
		{
			ShowImportModelWindow();
		}

		// --- 改名弹窗处理（如果正在编辑 Tag） ---
		if (m_IsEditingTag && m_EditingEntity != entt::null)
		{
			auto* activeScene = m_SceneManager ? m_SceneManager->GetActiveScene() : nullptr;
			if (!activeScene) {
				// 场景丢失或切换，取消编辑状态
				m_IsEditingTag = false;
				m_EditingEntity = entt::null;
				m_ShouldOpenRenamePopup = false;
				m_RenamePopupNeedsFocus = false;
			}
			else {
				// 如果上一帧请求打开 popup，则在这一帧打开（只调用一次）
				if (m_ShouldOpenRenamePopup) {
					ImGui::OpenPopup("Rename Entity");
					m_ShouldOpenRenamePopup = false;
					// m_RenamePopupNeedsFocus 已在触发处设为 true
				}

				// 现在尝试显示 modal（如果尚未打开则 BeginPopupModal 返回 false）
				if (ImGui::BeginPopupModal("Rename Entity", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
				{
					ImGui::Text("Enter new name:");
					ImGui::Separator();

					ImGui::SetNextItemWidth(350.0f);

					// 仅在弹窗刚打开时设置键盘焦点一次，避免干扰鼠标点击
					if (m_RenamePopupNeedsFocus) {
						ImGui::SetKeyboardFocusHere();
						m_RenamePopupNeedsFocus = false;
					}

					// InputText：保留 EnterReturnsTrue 支持按回车提交
					bool enterPressed = ImGui::InputText("##TagInput", m_TagEditBuffer, sizeof(m_TagEditBuffer), ImGuiInputTextFlags_EnterReturnsTrue);

					// OK / Cancel 按钮现在会响应鼠标点击
					if (enterPressed || ImGui::Button("OK", ImVec2(120, 0)))
					{
						std::string newTag = std::string(m_TagEditBuffer);
						// trim
						auto trim = [](std::string& s) {
							s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
							s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
							};
						trim(newTag);

						if (!newTag.empty()) {
							auto& ecs = activeScene->GetECS();
							if (ecs.HasComponent<TagComponent>(m_EditingEntity)) {
								ecs.GetComponent<TagComponent>(m_EditingEntity).Tag = newTag;
							}
							else {
								ecs.AddComponent<TagComponent>(m_EditingEntity, newTag);
							}

							if (m_EditingEntity == m_SelectedEntity) {
								m_SelectedEntityName = newTag;
							}

							// 同步列表以确保显示一致
							RefreshEntityList();
						}

						// 关闭弹窗与编辑状态
						m_IsEditingTag = false;
						m_EditingEntity = entt::null;
						ImGui::CloseCurrentPopup();
					}

					ImGui::SameLine();
					if (ImGui::Button("Cancel", ImVec2(120, 0)))
					{
						// 取消编辑
						m_IsEditingTag = false;
						m_EditingEntity = entt::null;
						m_RenamePopupNeedsFocus = false;
						ImGui::CloseCurrentPopup();
					}

					ImGui::EndPopup();
				} // if BeginPopupModal
			} // else activeScene valid
		} // if m_IsEditingTag



		// 现在渲染 ImGui（原有）
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

			if (ImGui::BeginMenu("Create Primitive"))
			{
				if (ImGui::MenuItem("Cube")) CreatePrimitive(ShapeType::Cube);
				if (ImGui::MenuItem("Sphere")) CreatePrimitive(ShapeType::Sphere);
				if (ImGui::MenuItem("Plane")) CreatePrimitive(ShapeType::Plane);
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Create Light"))
			{
				if (ImGui::MenuItem("Directional Light")) CreateLight(LightType::Directional);
				if (ImGui::MenuItem("Point Light")) CreateLight(LightType::Point);
				if (ImGui::MenuItem("Spot Light")) CreateLight(LightType::Spot);
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

	/**
	 * 绘制渲染视口
	 * 功能：显示场景渲染结果（从RendererLayer获取纹理）、检测尺寸变化、处理交互状态
	 */
	 // DrawViewport (替换你原来的实现)
	void ImGuiLayer::DrawViewport()
	{
		ImGui::Begin("Viewport");

		m_ViewportHovered = ImGui::IsWindowHovered();
		m_ViewportFocused = ImGui::IsWindowFocused();

		Application::Get().SetViewportState(m_ViewportHovered, m_ViewportFocused, m_IsUsingGizmo);

		ImVec2 avail = ImGui::GetContentRegionAvail();
		if (avail.x < 1.0f) avail.x = 1.0f;
		if (avail.y < 1.0f) avail.y = 1.0f;

		// 尺寸变化检测（保持你原逻辑）
		if (m_RendererLayer &&
			((uint32_t)avail.x != (uint32_t)m_LastViewportSize.x ||
				(uint32_t)avail.y != (uint32_t)m_LastViewportSize.y))
		{
			m_RendererLayer->ResizeViewport((uint32_t)avail.x, (uint32_t)avail.y);
			m_LastViewportSize = avail;
		}

		// 显示渲染纹理
		void* texID = nullptr;
		if (m_RendererLayer)
		{
			GLuint id = m_RendererLayer->GetSceneTextureID();
			texID = (void*)(intptr_t)id;
		}

		if (texID)
		{
			ImGui::Image(texID, avail, ImVec2(0, 1), ImVec2(1, 0));

			// **关键：使用 Image 的实际 rect 来设置视口偏移与尺寸**
			ImVec2 imageMin = ImGui::GetItemRectMin();
			ImVec2 imageMax = ImGui::GetItemRectMax();
			m_ViewportOffset = imageMin;
			m_ViewportSize = ImVec2(imageMax.x - imageMin.x, imageMax.y - imageMin.y);

			// **在这里直接绘制 gizmo（保证使用该窗口的 drawlist / rect）**
			if (m_SelectedEntity != entt::null)
				RenderGizmo();
		}
		else
		{
			ImGui::Text("No render target available");
			ImGui::Dummy(avail);
			m_ViewportSize = avail;
		}

		ImGui::End();
	}



	/**
	 * 显示实体管理器窗口
	 * 功能：列出场景中所有实体、支持选择和拖拽
	 */
	void ImGuiLayer::ShowEntityManagerWindow() {
		ImGui::Begin("Entity Manager");

		if (ImGui::Button("Refresh List")) {
			RefreshEntityList();
		}
		ImGui::Separator();

		auto* activeScene = m_SceneManager ? m_SceneManager->GetActiveScene() : nullptr;
		if (!activeScene) { ImGui::Text("No active scene"); ImGui::End(); return; }

		auto& registry = activeScene->GetECS().GetRegistry();
		for (auto entity : m_CachedEntities) {
			if (!registry.valid(entity)) continue;

			// 获取名称
			std::string name = "Entity " + std::to_string(static_cast<uint32_t>(entity));
			if (registry.any_of<TagComponent>(entity)) {
				name = registry.get<TagComponent>(entity).Tag;
			}

			ImGui::PushID(static_cast<uint32_t>(entity));
			bool isSelected = (m_SelectedEntity == entity);
			if (ImGui::Selectable(name.c_str(), isSelected)) {
				m_SelectedEntity = entity;
				m_SelectedEntityName = name;
			}

			// 更稳健的右键处理：检测当前项是否被右键点击
			if (ImGui::IsItemClicked(1)) { // right click
				m_EditingEntity = entity;
				m_IsEditingTag = true;
				// 初始化缓冲区
				if (registry.any_of<TagComponent>(entity)) {
					std::string t = registry.get<TagComponent>(entity).Tag;
					std::strncpy(m_TagEditBuffer, t.c_str(), sizeof(m_TagEditBuffer) - 1);
				}
				else {
					std::strncpy(m_TagEditBuffer, name.c_str(), sizeof(m_TagEditBuffer) - 1);
				}
				m_TagEditBuffer[sizeof(m_TagEditBuffer) - 1] = '\0';

				// 请求在下一帧打开 modal，并在打开后设置焦点一次
				m_ShouldOpenRenamePopup = true;
				m_RenamePopupNeedsFocus = true;
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

		ImGui::Separator();

		// ==== Gizmo 操作 ====
		ImGui::Text("Gizmo Operation:");
		if (ImGui::RadioButton("Translate (W)", m_GizmoOperation == ImGuizmo::TRANSLATE))
			m_GizmoOperation = ImGuizmo::TRANSLATE;
		ImGui::SameLine();
		if (ImGui::RadioButton("Rotate (E)", m_GizmoOperation == ImGuizmo::ROTATE))
			m_GizmoOperation = ImGuizmo::ROTATE;
		ImGui::SameLine();
		if (ImGui::RadioButton("Scale (R)", m_GizmoOperation == ImGuizmo::SCALE))
			m_GizmoOperation = ImGuizmo::SCALE;

		ImGui::Spacing();

		bool isLocal = (m_GizmoMode == ImGuizmo::LOCAL);
		if (ImGui::Checkbox("Local Space", &isLocal))
			m_GizmoMode = isLocal ? ImGuizmo::LOCAL : ImGuizmo::WORLD;

		ImGui::Separator();

		// ==== 快捷提示 ====
		ImGui::TextUnformatted("Shortcuts:");
		ImGui::BulletText("W - Translate");
		ImGui::BulletText("E - Rotate");
		ImGui::BulletText("R - Scale");
		ImGui::BulletText("Ctrl - Snap (move/rotate/scale)");

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

		auto model = std::make_shared<Model>(modelPath);
		if (model->GetMeshes().empty()) {
			ITR_ERROR("Failed to load model: {}", modelPath);
			return false;
		}

		auto entity = activeScene->GetECS().CreateEntity();
		activeScene->GetECS().GetRegistry().emplace<TagComponent>(entity, "model");
		activeScene->GetECS().GetRegistry().emplace<TransformComponent>(entity);
		activeScene->GetECS().GetRegistry().emplace<ModelComponent>(entity, model);
		activeScene->GetECS().GetRegistry().emplace<MaterialComponent>(entity, defaultMaterial);

		RefreshEntityList();
		return true;
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

		std::string name = "Primitive";
		std::vector<Vertex> verts;
		std::vector<unsigned int> inds;

		// 生成网格顶点与索引（根据 ShapeType 调用 ShapeGenerator）
		switch (type)
		{
		case ShapeType::Cube:
		{
			auto pair = ShapeGenerator::GenerateCube(1.0f); // 假设返回 std::pair<std::vector<Vertex>, std::vector<unsigned int>>
			verts = std::move(pair.first);
			inds = std::move(pair.second);
			name = "Cube";
			break;
		}
		case ShapeType::Sphere:
		{
			auto pair = ShapeGenerator::GenerateSphere(0.5f, 36, 18);
			verts = std::move(pair.first);
			inds = std::move(pair.second);
			name = "Sphere";
			break;
		}
		case ShapeType::Plane:
		{
			auto pair = ShapeGenerator::GeneratePlane(2.0f, 2.0f, 1, 1);
			verts = std::move(pair.first);
			inds = std::move(pair.second);
			name = "Plane";
			break;
		}
		default:
			return;
		}

		// 使用空的纹理列表创建 Mesh（你的 Mesh 构造需要 textures 参数）
		std::vector<std::shared_ptr<Intro::Texture>> emptyTextures;
		auto meshPtr = std::make_shared<Intro::Mesh>(std::move(verts), std::move(inds), std::move(emptyTextures));

		// 将 Mesh 放入 ECS（假设 activeScene->GetECS().CreateEntity() 返回 entt::entity
		// 且 registry.emplace<T>(entity, ...) 可用）
		auto entity = activeScene->GetECS().CreateEntity();
		auto& registry = activeScene->GetECS().GetRegistry();

		// 添加组件 —— 按你工程中组件的定义来写（下面使用了你之前给出的 MeshComponent 定义）
		registry.emplace<TagComponent>(entity, name);
		registry.emplace<TransformComponent>(entity); // 默认变换
		registry.emplace<MeshComponent>(entity, meshPtr);
		registry.emplace<MaterialComponent>(entity, defaultMaterial);

		// 如果你的编辑器有刷新实体列表的函数，调用它（方法名可能不同）
		RefreshEntityList();
	}

	void ImGuiLayer::CreateLight(LightType type) {
		auto* activeScene = m_SceneManager ? m_SceneManager->GetActiveScene() : nullptr;
		if (!activeScene) return;

		// 1. 创建实体并添加标签组件
		auto entity = activeScene->CreateEntity();
		std::string lightName;
		switch (type) {
		case LightType::Directional: lightName = "Directional Light"; break;
		case LightType::Point: lightName = "Point Light"; break;
		case LightType::Spot: lightName = "Spot Light"; break;
		}
		activeScene->GetECS().AddComponent<TagComponent>(entity, lightName);

		// 2. 添加变换组件（影响灯光位置/方向）
		TransformComponent transform;
		// 点光/聚光灯默认位置在相机前方，方便观察
		if (type != LightType::Directional) {
			transform.transform.position = glm::vec3(0.0f); // 相机前方5单位
		}
		activeScene->GetECS().AddComponent<TransformComponent>(entity, transform);

		// 3. 添加灯光组件并设置默认属性
		LightComponent light;
		light.Type = type;
		light.Color = glm::vec3(1.0f); // 白色光
		light.Intensity = 1.0f;
		// 聚光灯默认角度
		if (type == LightType::Spot) {
			light.SpotAngle = 30.0f;
			light.InnerSpotAngle = 20.0f;
			light.Range = 10.0f;
		}
		// 点光默认范围
		if (type == LightType::Point) {
			light.Range = 10.0f;
		}
		activeScene->GetECS().AddComponent<LightComponent>(entity, light);

		// 4. 自动选中新创建的灯光实体
		m_SelectedEntity = entity;
		m_SelectedEntityName = lightName;
		RefreshEntityList();
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

	// RenderGizmo (替换你原来的实现)
	void ImGuiLayer::RenderGizmo()
	{
		// 基本校验（保留）
		if (m_SelectedEntity == entt::null) { std::cout << "Gizmo Debug: No entity selected\n"; return; }
		if (!m_RendererLayer) { std::cout << "Gizmo Debug: No renderer layer\n"; return; }
		if (!m_SceneManager) { std::cout << "Gizmo Debug: No scene manager\n"; return; }

		auto* activeScene = m_SceneManager->GetActiveScene();
		if (!activeScene) { std::cout << "Gizmo Debug: No active scene\n"; return; }

		auto& registry = activeScene->GetECS().GetRegistry();
		if (!registry.valid(m_SelectedEntity)) { std::cout << "Gizmo Debug: Invalid entity\n"; m_SelectedEntity = entt::null; return; }
		if (!registry.all_of<TransformComponent>(m_SelectedEntity)) { std::cout << "Gizmo Debug: Entity has no TransformComponent\n"; return; }

		// 获取相机矩阵（确保这是渲染到纹理时使用的相机）
		auto camera = m_RendererLayer->GetCamera();
		glm::mat4 view = camera.GetViewMat();
		glm::mat4 proj = camera.GetProjectionMat();

		// 防止无效矩阵
		if (glm::determinant(view) == 0.0f || glm::determinant(proj) == 0.0f) { std::cout << "Gizmo Debug: Invalid camera matrices\n"; return; }

		// **非常重要：使用当前窗口的 drawlist，这样 ImGuizmo 会绘制在这个 viewport 窗口上**
		ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
		ImGuizmo::SetRect(m_ViewportOffset.x, m_ViewportOffset.y, m_ViewportSize.x, m_ViewportSize.y);

		// 获取实体变换并构建 model 矩阵
		auto& tc = registry.get<TransformComponent>(m_SelectedEntity);
		glm::mat4 model = glm::translate(glm::mat4(1.0f), tc.transform.position) *
			glm::toMat4(tc.transform.rotation) *
			glm::scale(glm::mat4(1.0f), tc.transform.scale);

		// snap 判定：使用 io.KeyCtrl（比 ImGui::IsKeyDown 更可靠）
		ImGuiIO& io = ImGui::GetIO();
		bool useSnap = io.KeyCtrl;
		float snapValues[3] = { 0.5f, 0.5f, 0.5f };
		if (m_GizmoOperation == ImGuizmo::ROTATE) { snapValues[0] = snapValues[1] = snapValues[2] = 15.0f; }
		else if (m_GizmoOperation == ImGuizmo::SCALE) { snapValues[0] = snapValues[1] = snapValues[2] = 0.1f; }

		// 调用 ImGuizmo
		ImGuizmo::Manipulate(
			glm::value_ptr(view),
			glm::value_ptr(proj),
			m_GizmoOperation,
			m_GizmoMode,
			glm::value_ptr(model),
			nullptr,
			useSnap ? snapValues : nullptr
		);

		m_IsUsingGizmo = ImGuizmo::IsUsing();
		m_IsOverGizmo = ImGuizmo::IsOver();
		std::cout << "Gizmo Debug: IsUsing=" << m_IsUsingGizmo << ", IsOver=" << m_IsOverGizmo << std::endl;

		Application::Get().SetViewportState(m_ViewportHovered, m_ViewportFocused, m_IsUsingGizmo);

		// 只有在正在操作时才分解并写回组件
		if (m_IsUsingGizmo)
		{
			float translation[3], rotationEulerDeg[3], scaleArr[3];
			ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(model), translation, rotationEulerDeg, scaleArr);

			glm::vec3 newTranslation(translation[0], translation[1], translation[2]);
			glm::vec3 newRotationDeg(rotationEulerDeg[0], rotationEulerDeg[1], rotationEulerDeg[2]);
			glm::vec3 newScale(scaleArr[0], scaleArr[1], scaleArr[2]);

			glm::vec3 newRotationRad = glm::radians(newRotationDeg);
			glm::quat newQuat = glm::quat(newRotationRad);

			// 更新实体变换
			tc.transform.position = newTranslation;
			tc.transform.rotation = newQuat;
			tc.transform.scale = newScale;

			// 同步编辑器面板
			m_TransformEditor.position = newTranslation;
			m_TransformEditor.rotation = newQuat;
			m_TransformEditor.scale = newScale;
			m_EulerAngles = newRotationDeg;

			std::cout << "Gizmo Debug: Applied transform\n";
		}
	}

	bool ImGuiLayer::ShouldBlockEvent()
	{
		ImGuiIO& io = ImGui::GetIO();

		// 如果 ImGui 想要捕获鼠标，并且鼠标不在视口内，则阻止事件
		if (io.WantCaptureMouse && !m_ViewportHovered) {
			return true;
		}

		// 如果正在使用 Gizmo，阻止事件
		if (m_IsUsingGizmo) {
			return true;
		}

		return false;
	}


	// 事件处理函数（保持原有事件系统逻辑，不修改返回值）
	bool ImGuiLayer::OnMouseButtonPressedEvent(MouseButtonPressedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MouseDown[e.GetMouseButton()] = true;

		return ShouldBlockEvent();
	}

	bool ImGuiLayer::OnMouseButtonReleasedEvent(MouseButtonReleasedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MouseDown[e.GetMouseButton()] = false;

		return ShouldBlockEvent();
	}

	bool ImGuiLayer::OnMouseMovedEvent(MouseMovedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MousePos = ImVec2(e.GetX(), e.GetY());

		return ShouldBlockEvent();
	}



	bool ImGuiLayer::OnMouseScrolledEvent(MouseScrolledEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MouseWheelH += e.GetXOffset();
		io.MouseWheel += e.GetYOffset();  // 更新ImGui滚轮状态


		return ShouldBlockEvent();
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

		// Gizmo快捷键（仅在视口聚焦时）
		if (m_ViewportFocused && !io.WantCaptureKeyboard)
		{
			switch (e.GetKeyCode())
			{
			case GLFW_KEY_W:
				m_GizmoOperation = ImGuizmo::TRANSLATE;
				std::cout << "Gizmo: Switch to TRANSLATE" << std::endl;
				break;
			case GLFW_KEY_E:
				m_GizmoOperation = ImGuizmo::ROTATE;
				std::cout << "Gizmo: Switch to ROTATE" << std::endl;
				break;
			case GLFW_KEY_R:
				m_GizmoOperation = ImGuizmo::SCALE;
				std::cout << "Gizmo: Switch to SCALE" << std::endl;
				break;
			case GLFW_KEY_T:
				// 切换本地/世界空间
				m_GizmoMode = (m_GizmoMode == ImGuizmo::LOCAL) ? ImGuizmo::WORLD : ImGuizmo::LOCAL;
				std::cout << "Gizmo: Switch mode to " << (m_GizmoMode == ImGuizmo::LOCAL ? "LOCAL" : "WORLD") << std::endl;
				break;
			}
		}

		// 键盘按下事件：ImGui捕获时阻止传递
		return ShouldBlockEvent();
	}

	bool ImGuiLayer::OnKeyReleasedEvent(KeyReleasedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeysDown[e.GetKeyCode()] = false;  // 更新ImGui按键状态

		// 键盘释放事件：ImGui捕获时阻止传递
		return ShouldBlockEvent();
	}

	bool ImGuiLayer::OnKeyTypedEvent(KeyTypedEvent& e)
	{
		//ImGuiIO& io = ImGui::GetIO();
		//io.AddInputCharacter(e.GetKeyCode());  // 传递输入字符给ImGui

		// 字符输入事件：ImGui捕获时阻止传递
		return ShouldBlockEvent();
	}

	bool ImGuiLayer::OnWindowResizedEvent(WindowResizeEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(e.GetWidth(), e.GetHeight());
		io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);  // 更新ImGui窗口尺寸

		// 窗口 resize 事件通常需要传递给其他层（如RendererLayer更新视口），保持返回false
		return ShouldBlockEvent();
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