﻿// ImGuiLayer.cpp
#include "itrpch.h"
#include "ImGuiLayer.h"

#include "Intro/Renderer/Renderer.h"
#include "Intro/Config/ConfigObserver.h"
#include "Intro/Application.h"
#include "Intro/Renderer/ShapeGenerator.h"
#include "Intro/RecourceManager/ResourceManager.h"
#include "Platform/OpenGL/ImGuiOpenGLRenderer.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_glfw.h"
#include "misc/cpp/imgui_stdlib.h"
#include "entt/entt.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/quaternion.hpp>

#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Intro {

	// -------------------------------------------------------------------------
	// Construction / Destruction
	// -------------------------------------------------------------------------
	ImGuiLayer::ImGuiLayer(SceneManager* sceneManager, RendererLayer* rendererLayer)
		: Layer("ImGuiLayer")
		, m_SceneManager(sceneManager)
		, m_RendererLayer(rendererLayer)
	{
		// 不在构造时尝试加载资源（避免硬编码路径）
	}

	ImGuiLayer::~ImGuiLayer()
	{
		// 确保释放上下文（OnDetach 也会做）
	}

	// -------------------------------------------------------------------------
	// Lifecycle helpers
	// -------------------------------------------------------------------------
	void ImGuiLayer::OnAttach()
	{
		InitImGui();
		RefreshEntityList();
	}

	void ImGuiLayer::OnDetach()
	{
		ShutdownImGui();
	}

	void ImGuiLayer::InitImGui()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		ImGui::StyleColorsDark();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGuiStyle& style = ImGui::GetStyle();
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		GLFWwindow* window = (GLFWwindow*)Application::Get().GetWindow().GetNativeWindow();
		IM_ASSERT(window);
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");

		ImGuizmo::SetImGuiContext(ImGui::GetCurrentContext());
	}

	void ImGuiLayer::ShutdownImGui()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	// -------------------------------------------------------------------------
	// Frame management
	// -------------------------------------------------------------------------
	void ImGuiLayer::BeginFrame()
	{
		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();

		ImGuizmo::BeginFrame();
	}

	void ImGuiLayer::EndFrameAndRender()
	{
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}

	// -------------------------------------------------------------------------
	// Main update (called once per frame)
	// -------------------------------------------------------------------------
	void ImGuiLayer::OnUpdate(float /*deltaTime*/)
	{
		// Update ImGui IO timing and display size
		ImGuiIO& io = ImGui::GetIO();
		auto& app = Application::Get();
		io.DisplaySize = ImVec2(app.GetWindow().GetWidth(), app.GetWindow().GetHeight());

		float time = (float)glfwGetTime();
		io.DeltaTime = m_Time > 0.0f ? (time - m_Time) : (1.0f / 60.0f);
		m_Time = time;

		BeginFrame();

		// Draw UI
		DrawMenuBar();
		DrawDockSpaceHost();
		DrawViewport();
		ShowEntityManagerWindow();
		ShowSceneControlsWindow();

		if (m_ShowResourceBrowser) {
			ShowResourceBrowserWindow();
		}

		if (m_SelectedEntity != entt::null)
		{
			SyncTransformEditor();
			ShowEntityInspectorWindow();
		}

		if (m_ShowImportWindow) ShowImportModelWindow();

		HandleRenamePopup();

		EndFrameAndRender();
	}

	// -------------------------------------------------------------------------
	// Dock / menu / main windows
	// -------------------------------------------------------------------------
	void ImGuiLayer::DrawDockSpaceHost()
	{
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
		ImGuiViewport* viewport = ImGui::GetMainViewport();

		const float menuBarHeight = 20.0f;

		ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + menuBarHeight));
		ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, viewport->Size.y - menuBarHeight));
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGuiWindowFlags host_window_flags = ImGuiWindowFlags_NoTitleBar;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

		ImGui::Begin("DockSpaceHost", nullptr, host_window_flags);
		ImGui::PopStyleVar(3);

		ImGuiID dockspace_id = ImGui::GetID("EditorDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

		ImGui::End();
	}

	void ImGuiLayer::DrawMenuBar()
	{
		if (!ImGui::BeginMainMenuBar()) return;

		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Import Model...")) m_ShowImportWindow = true;
			ImGui::Separator();
			if (ImGui::MenuItem("Save Config")) {
				Config::Get().Save();
			}
			if (ImGui::MenuItem("Reload Config")) {
				Config::Get().Load();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View")) {
			//if (ImGui::MenuItem("Renderer Settings")) m_ShowRendererSettings = true;
			//if (ImGui::MenuItem("Resource Manager")) m_ShowResourceManager = true;
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Create"))
		{
			if (ImGui::BeginMenu("Primitive"))
			{
				if (ImGui::MenuItem("Cube")) CreatePrimitive(ShapeType::Cube);
				if (ImGui::MenuItem("Sphere")) CreatePrimitive(ShapeType::Sphere);
				if (ImGui::MenuItem("Plane")) CreatePrimitive(ShapeType::Plane);
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Light"))
			{
				if (ImGui::MenuItem("Directional Light")) CreateLight(LightType::Directional);
				if (ImGui::MenuItem("Point Light")) CreateLight(LightType::Point);
				if (ImGui::MenuItem("Spot Light")) CreateLight(LightType::Spot);
				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	// -------------------------------------------------------------------------
	// Viewport (render texture)
	// -------------------------------------------------------------------------
	void ImGuiLayer::DrawViewport()
	{
		ImGui::Begin("Viewport");

		m_ViewportHovered = ImGui::IsWindowHovered();
		m_ViewportFocused = ImGui::IsWindowFocused();

		Application::Get().SetViewportState(m_ViewportHovered, m_ViewportFocused, m_IsUsingGizmo);

		ImVec2 avail = ImGui::GetContentRegionAvail();
		avail.x = std::max(avail.x, 1.0f);
		avail.y = std::max(avail.y, 1.0f);

		// 检测大小变化并通知 RendererLayer
		if (m_RendererLayer &&
			((uint32_t)avail.x != (uint32_t)m_LastViewportSize.x ||
				(uint32_t)avail.y != (uint32_t)m_LastViewportSize.y))
		{
			m_RendererLayer->ResizeViewport((uint32_t)avail.x, (uint32_t)avail.y);
			m_LastViewportSize = avail;
		}

		void* texID = nullptr;
		if (m_RendererLayer)
		{
			GLuint id = m_RendererLayer->GetSceneTextureID();
			texID = (void*)(intptr_t)id;
		}

		if (texID)
		{
			ImGui::Image(texID, avail, ImVec2(0, 1), ImVec2(1, 0));
			ImVec2 imageMin = ImGui::GetItemRectMin();
			ImVec2 imageMax = ImGui::GetItemRectMax();
			m_ViewportOffset = imageMin;
			m_ViewportSize = ImVec2(imageMax.x - imageMin.x, imageMax.y - imageMin.y);

			// 在视口内绘制 gizmo（如果选中实体）
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

	// -------------------------------------------------------------------------
	// Entity manager / inspector / scene controls
	// -------------------------------------------------------------------------
	void ImGuiLayer::ShowEntityManagerWindow()
	{
		ImGui::Begin("Entity Manager");

		if (ImGui::Button("Refresh List")) RefreshEntityList();
		ImGui::Separator();

		auto* activeScene = m_SceneManager ? m_SceneManager->GetActiveScene() : nullptr;
		if (!activeScene) { ImGui::Text("No active scene"); ImGui::End(); return; }

		auto& registry = activeScene->GetECS().GetRegistry();
		for (auto entity : m_CachedEntities)
		{
			if (!registry.valid(entity)) continue;

			std::string name = "Entity " + std::to_string(static_cast<uint32_t>(entity));
			if (registry.any_of<TagComponent>(entity))
				name = registry.get<TagComponent>(entity).Tag;

			ImGui::PushID(static_cast<uint32_t>(entity));
			bool isSelected = (m_SelectedEntity == entity);
			if (ImGui::Selectable(name.c_str(), isSelected))
			{
				m_SelectedEntity = entity;
				m_SelectedEntityName = name;
			}

			if (ImGui::IsItemClicked(1))
			{
				m_EditingEntity = entity;
				m_IsEditingTag = true;
				if (registry.any_of<TagComponent>(entity))
				{
					std::string t = registry.get<TagComponent>(entity).Tag;
					std::strncpy(m_TagEditBuffer, t.c_str(), sizeof(m_TagEditBuffer) - 1);
				}
				else
				{
					std::strncpy(m_TagEditBuffer, name.c_str(), sizeof(m_TagEditBuffer) - 1);
				}
				m_TagEditBuffer[sizeof(m_TagEditBuffer) - 1] = '\0';
				m_ShouldOpenRenamePopup = true;
				m_RenamePopupNeedsFocus = true;
			}

			ImGui::PopID();
		}

		ImGui::End();
	}

	void ImGuiLayer::ShowEntityInspectorWindow()
	{
		ImGui::Begin("Inspector");

		if (m_SelectedEntity == entt::null) { ImGui::Text("No entity selected"); ImGui::End(); return; }
		auto* activeScene = m_SceneManager ? m_SceneManager->GetActiveScene() : nullptr;
		if (!activeScene) { ImGui::End(); return; }

		auto& registry = activeScene->GetECS().GetRegistry();
		if (!registry.valid(m_SelectedEntity)) { m_SelectedEntity = entt::null; ImGui::End(); return; }

		ImGui::Text("Entity: %s", m_SelectedEntityName.c_str());
		ImGui::Separator();

		if (registry.any_of<TransformComponent>(m_SelectedEntity))
		{
			if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
			{
				bool transformChanged = false;

				// Position
				glm::vec3 oldPosition = m_TransformEditor.position;
				ImGui::DragFloat3("Position", &m_TransformEditor.position.x, 0.1f);
				if (m_TransformEditor.position != oldPosition) {
					transformChanged = true;
				}

				// Rotation (Euler angles)
				glm::vec3 oldEuler = m_EulerAngles;
				ImGui::DragFloat3("Rotation", &m_EulerAngles.x, 1.0f);
				if (m_EulerAngles != oldEuler) {
					m_TransformEditor.rotation = glm::quat(glm::radians(m_EulerAngles));
					transformChanged = true;
				}

				// Scale
				glm::vec3 oldScale = m_TransformEditor.scale;
				ImGui::DragFloat3("Scale", &m_TransformEditor.scale.x, 0.1f);
				if (m_TransformEditor.scale != oldScale) {
					transformChanged = true;
				}

				// 实时更新变换
				if (transformChanged) {
					UpdateSelectedEntityTransform();
				}

				// 保留按钮作为备用
				if (ImGui::Button("Reset Transform"))
				{
					m_TransformEditor.position = glm::vec3(0.0f);
					m_EulerAngles = glm::vec3(0.0f);
					m_TransformEditor.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
					m_TransformEditor.scale = glm::vec3(1.0f);
					UpdateSelectedEntityTransform();
				}
			}
		}

		if (registry.any_of<LightComponent>(m_SelectedEntity))
		{
			if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
			{
				auto& light = registry.get<LightComponent>(m_SelectedEntity);
				const char* types[] = { "Directional", "Point", "Spot" };
				int cur = (int)light.Type;
				if (ImGui::Combo("Type", &cur, types, 3)) light.Type = (LightType)cur;

				ImGui::ColorEdit3("Color", &light.Color.r);
				ImGui::DragFloat("Intensity", &light.Intensity, 0.1f, 0.0f, 10.0f);
				if (light.Type == LightType::Directional)
				{
					ImGui::Text("Direction controlled by entity rotation");
					if (ImGui::Button("Down")) { auto& t = registry.get<TransformComponent>(m_SelectedEntity); t.transform.rotation = glm::quat(1.0f, 0, 0, 0); SyncTransformEditor(); }
					ImGui::SameLine();
					if (ImGui::Button("Up")) { auto& t = registry.get<TransformComponent>(m_SelectedEntity); t.transform.rotation = glm::angleAxis(glm::radians(180.0f), glm::vec3(1, 0, 0)); SyncTransformEditor(); }
					ImGui::SameLine();
					if (ImGui::Button("Forward")) { auto& t = registry.get<TransformComponent>(m_SelectedEntity); t.transform.rotation = glm::angleAxis(glm::radians(-90.0f), glm::vec3(1, 0, 0)); SyncTransformEditor(); }
				}

				if (light.Type == LightType::Point || light.Type == LightType::Spot)
					ImGui::DragFloat("Range", &light.Range, 0.1f, 0.1f, 100.0f);

				if (light.Type == LightType::Spot)
				{
					ImGui::DragFloat("Spot Angle", &light.SpotAngle, 1.0f, 1.0f, 89.0f);
					ImGui::DragFloat("Inner Angle", &light.InnerSpotAngle, 1.0f, 1.0f, light.SpotAngle);
				}
			}
		}

		ImGui::End();
	}

	void ImGuiLayer::ShowSceneControlsWindow()
	{
		ImGui::Begin("Scene Controls");

		if (ImGui::Button("Clear Scene") && m_SceneManager)
		{
			// 如果你实现了 ClearActiveScene，放开注释
			// m_SceneManager->ClearActiveScene();
		}
		ImGui::Separator();

		ImGui::Text("Gizmo Operation:");
		if (ImGui::RadioButton("Translate (W)", m_GizmoOperation == ImGuizmo::TRANSLATE)) m_GizmoOperation = ImGuizmo::TRANSLATE;
		ImGui::SameLine();
		if (ImGui::RadioButton("Rotate (E)", m_GizmoOperation == ImGuizmo::ROTATE)) m_GizmoOperation = ImGuizmo::ROTATE;
		ImGui::SameLine();
		if (ImGui::RadioButton("Scale (R)", m_GizmoOperation == ImGuizmo::SCALE)) m_GizmoOperation = ImGuizmo::SCALE;

		bool isLocal = (m_GizmoMode == ImGuizmo::LOCAL);
		if (ImGui::Checkbox("Local Space", &isLocal)) m_GizmoMode = isLocal ? ImGuizmo::LOCAL : ImGuizmo::WORLD;

		ImGui::Separator();
		ImGui::TextUnformatted("Shortcuts:");
		ImGui::BulletText("W - Translate");
		ImGui::BulletText("E - Rotate");
		ImGui::BulletText("R - Scale");
		ImGui::BulletText("Ctrl - Snap (move/rotate/scale)");

		ImGui::End();
	}

	// -------------------------------------------------------------------------
	// Import model window
	// -------------------------------------------------------------------------
	void ImGuiLayer::ShowImportModelWindow()
	{
		ImGui::Begin("Import Model", &m_ShowImportWindow);

		ImGui::InputText("Model Path", &m_ModelImportPath);
		ImGui::SameLine();
		if (ImGui::Button("Browse"))
		{
			// 文件对话框：由你选择的库实现（nfd 等）
		}

		if (ImGui::Button("Import"))
		{
			if (!m_ModelImportPath.empty())
			{
				if (ImportModel(m_ModelImportPath))
					m_ShowImportWindow = false;
			}
		}

		ImGui::End();
	}

	// -------------------------------------------------------------------------
	// Rename modal handling
	// -------------------------------------------------------------------------
	void ImGuiLayer::HandleRenamePopup()
	{
		if (!m_IsEditingTag || m_EditingEntity == entt::null) return;

		auto* activeScene = m_SceneManager ? m_SceneManager->GetActiveScene() : nullptr;
		if (!activeScene)
		{
			m_IsEditingTag = false;
			m_EditingEntity = entt::null;
			m_ShouldOpenRenamePopup = false;
			m_RenamePopupNeedsFocus = false;
			return;
		}

		if (m_ShouldOpenRenamePopup)
		{
			ImGui::OpenPopup("Rename Entity");
			m_ShouldOpenRenamePopup = false;
		}

		if (ImGui::BeginPopupModal("Rename Entity", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Enter new name:");
			ImGui::Separator();
			ImGui::SetNextItemWidth(350.0f);

			if (m_RenamePopupNeedsFocus)
			{
				ImGui::SetKeyboardFocusHere();
				m_RenamePopupNeedsFocus = false;
			}

			bool enterPressed = ImGui::InputText("##TagInput", m_TagEditBuffer, sizeof(m_TagEditBuffer), ImGuiInputTextFlags_EnterReturnsTrue);

			if (enterPressed || ImGui::Button("OK", ImVec2(120, 0)))
			{
				std::string newTag = std::string(m_TagEditBuffer);
				// trim
				auto trim = [](std::string& s) {
					s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
					s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
					};
				trim(newTag);

				if (!newTag.empty())
				{
					auto& ecs = activeScene->GetECS();
					if (ecs.HasComponent<TagComponent>(m_EditingEntity))
						ecs.GetComponent<TagComponent>(m_EditingEntity).Tag = newTag;
					else
						ecs.AddComponent<TagComponent>(m_EditingEntity, newTag);

					if (m_EditingEntity == m_SelectedEntity) m_SelectedEntityName = newTag;
					RefreshEntityList();
				}

				m_IsEditingTag = false;
				m_EditingEntity = entt::null;
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				m_IsEditingTag = false;
				m_EditingEntity = entt::null;
				m_RenamePopupNeedsFocus = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	// -------------------------------------------------------------------------
	// Gizmo
	// -------------------------------------------------------------------------
	void ImGuiLayer::RenderGizmo()
	{
		// 基本检查
		if (m_SelectedEntity == entt::null || !m_RendererLayer || !m_SceneManager) return;

		auto* activeScene = m_SceneManager->GetActiveScene();
		if (!activeScene) return;

		auto& registry = activeScene->GetECS().GetRegistry();
		if (!registry.valid(m_SelectedEntity)) { m_SelectedEntity = entt::null; return; }
		if (!registry.any_of<TransformComponent>(m_SelectedEntity)) return;

		auto camera = m_RendererLayer->GetCamera();
		glm::mat4 view = camera.GetViewMat();
		glm::mat4 proj = camera.GetProjectionMat();

		// 设置 ImGuizmo 对应 drawlist 与 rect
		ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
		ImGuizmo::SetRect(m_ViewportOffset.x, m_ViewportOffset.y, m_ViewportSize.x, m_ViewportSize.y);

		auto& tc = registry.get<TransformComponent>(m_SelectedEntity);
		glm::mat4 model = glm::translate(glm::mat4(1.0f), tc.transform.position) *
			glm::toMat4(tc.transform.rotation) *
			glm::scale(glm::mat4(1.0f), tc.transform.scale);

		// snap 判定（使用 ImGui IO）
		ImGuiIO& io = ImGui::GetIO();
		bool useSnap = io.KeyCtrl;
		float snapValues[3] = { 0.5f, 0.5f, 0.5f };
		if (m_GizmoOperation == ImGuizmo::ROTATE) { snapValues[0] = snapValues[1] = snapValues[2] = 15.0f; }
		else if (m_GizmoOperation == ImGuizmo::SCALE) { snapValues[0] = snapValues[1] = snapValues[2] = 0.1f; }

		ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(proj),
			m_GizmoOperation, m_GizmoMode,
			glm::value_ptr(model), nullptr, useSnap ? snapValues : nullptr);

		m_IsUsingGizmo = ImGuizmo::IsUsing();
		m_IsOverGizmo = ImGuizmo::IsOver();

		Application::Get().SetViewportState(m_ViewportHovered, m_ViewportFocused, m_IsUsingGizmo);

		if (m_IsUsingGizmo)
		{
			float translation[3], rotationEulerDeg[3], scaleArr[3];
			ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(model), translation, rotationEulerDeg, scaleArr);

			glm::vec3 newTranslation(translation[0], translation[1], translation[2]);
			glm::vec3 newScale(scaleArr[0], scaleArr[1], scaleArr[2]);
			glm::quat newQuat = glm::quat_cast(glm::mat3(model));

			auto& comp = registry.get<TransformComponent>(m_SelectedEntity);
			comp.transform.position = newTranslation;
			comp.transform.rotation = newQuat;
			comp.transform.scale = newScale;

			m_TransformEditor.position = newTranslation;
			m_TransformEditor.rotation = newQuat;
			m_TransformEditor.scale = newScale;
			m_EulerAngles = glm::degrees(glm::eulerAngles(newQuat));
		}
	}

	// -------------------------------------------------------------------------
	// Utilities: entity list, import, create primitives/lights, transform sync
	// -------------------------------------------------------------------------
	void ImGuiLayer::RefreshEntityList()
	{
		m_CachedEntities.clear();
		auto* activeScene = m_SceneManager ? m_SceneManager->GetActiveScene() : nullptr;
		if (!activeScene) return;

		auto& registry = activeScene->GetECS().GetRegistry();
		std::unordered_set<entt::entity> set;
		auto view = registry.view<TransformComponent>();
		for (auto e : view) set.insert(e);
		m_CachedEntities.assign(set.begin(), set.end());
	}

	bool ImGuiLayer::ImportModel(const std::string& modelPath)
	{
		if (!m_SceneManager) return false;
		auto* activeScene = m_SceneManager->GetActiveScene();
		if (!activeScene) return false;

		auto model = ResourceManager::Get().LoadModel(modelPath);
		if (model->GetMeshes().empty()) {
			ITR_ERROR("Failed to load model: {}", modelPath);
			return false;
		}

		auto entity = activeScene->GetECS().CreateEntity();
		auto& reg = activeScene->GetECS().GetRegistry();
		reg.emplace<TagComponent>(entity, "model");
		reg.emplace<TransformComponent>(entity);
		reg.emplace<ModelComponent>(entity, model);
		reg.emplace<MaterialComponent>(entity, m_DefaultMaterial ? m_DefaultMaterial : std::shared_ptr<Material>());

		RefreshEntityList();
		return true;
	}

	void ImGuiLayer::CreatePrimitive(ShapeType type)
	{
		if (!m_SceneManager) return;
		auto* activeScene = m_SceneManager->GetActiveScene();
		if (!activeScene) return;

		std::string name = "Primitive";
		std::vector<Vertex> verts;
		std::vector<unsigned int> inds;

		switch (type) {
		case ShapeType::Cube: {
			auto p = ShapeGenerator::GenerateCube(1.0f);
			verts = std::move(p.first); inds = std::move(p.second); name = "Cube"; break;
		}
		case ShapeType::Sphere: {
			auto p = ShapeGenerator::GenerateSphere(0.5f, 36, 18);
			verts = std::move(p.first); inds = std::move(p.second); name = "Sphere"; break;
		}
		case ShapeType::Plane: {
			auto p = ShapeGenerator::GeneratePlane(2.0f, 2.0f, 1, 1);
			verts = std::move(p.first); inds = std::move(p.second); name = "Plane"; break;
		}
		default: return;
		}

		std::vector<std::shared_ptr<Texture>> emptyTextures;
		auto meshPtr = std::make_shared<Mesh>(std::move(verts), std::move(inds), std::move(emptyTextures));

		auto entity = activeScene->GetECS().CreateEntity();
		auto& reg = activeScene->GetECS().GetRegistry();
		reg.emplace<TagComponent>(entity, name);
		reg.emplace<TransformComponent>(entity);
		reg.emplace<MeshComponent>(entity, meshPtr);
		reg.emplace<MaterialComponent>(entity, m_DefaultMaterial ? m_DefaultMaterial : std::shared_ptr<Material>());

		RefreshEntityList();
	}

	void ImGuiLayer::CreateLight(LightType type)
	{
		auto* activeScene = m_SceneManager ? m_SceneManager->GetActiveScene() : nullptr;
		if (!activeScene || !m_RendererLayer) return;

		auto camera = m_RendererLayer->GetCamera();
		glm::vec3 camPos = camera.GetPosition();
		glm::vec3 camForward = camera.GetFront();

		auto entity = activeScene->CreateEntity();
		std::string lightName;
		LightComponent light;
		light.Type = type;
		light.Color = glm::vec3(1.0f);

		switch (type) {
		case LightType::Directional:
			lightName = "Directional Light";
			light.Intensity = 1.0f;
			light.Direction = glm::vec3(0.0f, -1.0f, 0.0f);
			break;
		case LightType::Point:
			lightName = "Point Light";
			light.Intensity = 2.0f;
			light.Range = 10.0f;
			break;
		case LightType::Spot:
			lightName = "Spot Light";
			light.Intensity = 3.0f;
			light.Range = 15.0f;
			light.SpotAngle = 45.0f;
			light.InnerSpotAngle = 30.0f;
			light.Direction = glm::vec3(0.0f, 0.0f, -1.0f);
			break;
		}

		activeScene->GetECS().AddComponent<TagComponent>(entity, lightName);

		TransformComponent tc;
		if (type == LightType::Directional) {
			tc.transform.position = glm::vec3(0.0f, 1.0f, 0.0f);
			tc.transform.rotation = glm::angleAxis(glm::radians(-45.0f), glm::vec3(1, 0, 0));
		}
		else {
			tc.transform.position = camPos + camForward * 5.0f;
		}
		activeScene->GetECS().AddComponent<TransformComponent>(entity, tc);
		activeScene->GetECS().AddComponent<LightComponent>(entity, light);

		m_SelectedEntity = entity;
		m_SelectedEntityName = lightName;
		SyncTransformEditor();
		RefreshEntityList();

		ITR_INFO("Created {} at ({:.2f},{:.2f},{:.2f})", lightName, tc.transform.position.x, tc.transform.position.y, tc.transform.position.z);
	}

	// ImGuiLayer.cpp - 添加渲染设置窗口
	void ImGuiLayer::ShowRendererSettingsWindow() {
		ImGui::Begin("Renderer Settings");

		Config& config = Config::Get();
		auto& graphicsConfig = config.GetGraphicsConfig();
		bool configChanged = false;

		// MSAA 设置
		if (ImGui::Checkbox("Enable MSAA", &graphicsConfig.EnableMSAA)) {
			configChanged = true;
		}

		if (graphicsConfig.EnableMSAA) {
			const char* msaaItems[] = { "MSAA 2x", "MSAA 4x", "MSAA 8x" };
			int currentMSAA = 0;
			if (graphicsConfig.MSaaSamples == 2) currentMSAA = 0;
			else if (graphicsConfig.MSaaSamples == 4) currentMSAA = 1;
			else if (graphicsConfig.MSaaSamples == 8) currentMSAA = 2;

			if (ImGui::Combo("MSAA Samples", &currentMSAA, msaaItems, IM_ARRAYSIZE(msaaItems))) {
				graphicsConfig.MSaaSamples = (currentMSAA == 0) ? 2 : (currentMSAA == 1) ? 4 : 8;
				configChanged = true;
			}
		}

		// HDR 设置
		if (ImGui::Checkbox("Enable HDR", &graphicsConfig.EnableHDR)) {
			configChanged = true;
		}

		// Gamma 校正
		if (ImGui::Checkbox("Enable Gamma Correction", &graphicsConfig.EnableGammaCorrection)) {
			configChanged = true;
		}

		// 后期处理设置
		if (ImGui::CollapsingHeader("Post Processing")) {
			if (ImGui::Checkbox("Enable Post Processing", &graphicsConfig.EnablePostProcessing)) {
				configChanged = true;
			}

			if (graphicsConfig.EnablePostProcessing) {
				if (ImGui::SliderFloat("Bloom Threshold", &graphicsConfig.BloomThreshold, 0.5f, 2.0f)) {
					configChanged = true;
				}
				if (ImGui::SliderFloat("Bloom Intensity", &graphicsConfig.BloomIntensity, 0.0f, 2.0f)) {
					configChanged = true;
				}
				if (ImGui::SliderFloat("Vignette Intensity", &graphicsConfig.VignetteIntensity, 0.0f, 1.0f)) {
					configChanged = true;
				}
				if (ImGui::SliderFloat("Chromatic Aberration", &graphicsConfig.ChromaticAberration, 0.0f, 0.1f)) {
					configChanged = true;
				}
			}
		}

		// 统计信息
		if (ImGui::CollapsingHeader("Statistics")) {
			auto stats = Renderer::GetStats();
			ImGui::Text("Draw Calls: %d", stats.drawCalls);
			ImGui::Text("Triangles: %d", stats.triangleCount);
			ImGui::Text("Vertices: %d", stats.vertexCount);

			if (ImGui::Button("Reset Stats")) {
				Renderer::ResetStats();
			}
		}

		if (configChanged) {
			config.MarkDirty(true);
			ConfigObserver::Get().OnGraphicsConfigChanged(graphicsConfig);
		}

		ImGui::End();
	}

	// 实现资源浏览器窗口
// --- Replace existing ShowResourceBrowserWindow and DrawFileTreeNode implementations with this block ---

	static const char* SortModeNames[] = { "Name", "Date", "Type" };

	void ImGuiLayer::ShowResourceBrowserWindow() {
		ImGui::Begin("Resource Browser", &m_ShowResourceBrowser);

		auto& resourceManager = ResourceManager::Get();
		auto fileTree = resourceManager.GetFileTree();

		// Toolbar: Refresh / Import / New Folder / Search / Sort
		// Note: ImGui doesn't have BeginHorizontal by default; use SameLine() for horizontal layout.
		if (ImGui::Button("Refresh")) {
			resourceManager.RefreshFileTree();
			fileTree = resourceManager.GetFileTree();
		}
		ImGui::SameLine();
		if (ImGui::Button("Import")) {
			m_ShowImportWindow = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("New Folder")) {
			if (fileTree) {
				std::string newName = "NewFolder";
				std::string basePath = fileTree->info.path;
				std::filesystem::path p(basePath);
				std::filesystem::path newDir = p / newName;
				int counter = 1;
				while (std::filesystem::exists(newDir)) {
					newDir = p / (newName + std::to_string(counter++));
				}
				std::error_code ec;
				std::filesystem::create_directories(newDir, ec);
				resourceManager.RefreshFileTree();
			}
		}

		ImGui::SameLine();
		ImGui::TextUnformatted("Search:");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200.0f);
		ImGui::InputText("##ResourceSearch", &m_ResourceSearch);

		ImGui::SameLine();
		ImGui::Checkbox("Show hidden", &m_ShowHiddenFiles);
		ImGui::SameLine();
		ImGui::Text("Sort:");
		ImGui::SameLine();
		static const char* SortModeNames[] = { "Name", "Date", "Type" };
		ImGui::Combo("##SortMode", &m_SortMode, SortModeNames, IM_ARRAYSIZE(SortModeNames));

		ImGui::Separator();

		if (!fileTree) {
			ImGui::Text("No file tree available. Click Refresh to scan.");
			ImGui::End();
			return;
		}

		// Prepare a recursive search function (std::function so it can be recursive)
		std::function<bool(std::shared_ptr<ResourceFileNode>)> matchesSearch =
			[&](std::shared_ptr<ResourceFileNode> n) -> bool {
			if (!n) return false;
			if (m_ResourceSearch.empty()) return true;
			std::string name = n->info.name;
			std::string lowerName = name;
			std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
			std::string lowerSearch = m_ResourceSearch;
			std::transform(lowerSearch.begin(), lowerSearch.end(), lowerSearch.begin(), ::tolower);
			if (lowerName.find(lowerSearch) != std::string::npos) return true;
			for (auto& c : n->children) {
				if (matchesSearch(c)) return true;
			}
			return false;
			};

		// Layout: left tree pane + right details pane
		ImGui::Columns(2, "ResColumns", true);
		ImGui::SetColumnWidth(0, m_ResourcePaneWidth);

		// Left pane: tree
		ImGui::BeginChild("ResourceTreePane", ImVec2(0, 0), true);
		// Breadcrumb
		ImGui::Text("Root: %s", fileTree->info.path.c_str());
		ImGui::Separator();

		// recursive draw
		std::function<void(std::shared_ptr<ResourceFileNode>)> drawNode;
		drawNode = [&](std::shared_ptr<ResourceFileNode> node) {
			if (!node) return;

			// filter by search (show node if name contains search or any child contains)
			if (!matchesSearch(node)) return;

			// Choose flags: leaf if no children
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
			if (node->children.empty()) flags |= ImGuiTreeNodeFlags_Leaf;
			if (m_SelectedResourceNode == node) flags |= ImGuiTreeNodeFlags_Selected;

			// Icon for type
			const char* icon = "📁";
			switch (node->info.type) {
			case ResourceType::Texture: icon = "🖼️"; break;
			case ResourceType::Model: icon = "🎯"; break;
			case ResourceType::Shader: icon = "🔮"; break;
			case ResourceType::Material: icon = "🎨"; break;
			case ResourceType::Scene: icon = "🌍"; break;
			default: icon = node->children.empty() ? "📄" : "📁"; break;
			}

			std::string label = std::string(icon) + " " + node->info.name;

			bool nodeOpen = ImGui::TreeNodeEx((void*)node.get(), flags, "%s", label.c_str());

			// click / selection
			if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
				m_SelectedResourceNode = node;
				// Double-click open action
				if (ImGui::IsMouseDoubleClicked(0)) {
					switch (node->info.type) {
					case ResourceType::Model:
						CreateModelEntity(node);
						break;
					case ResourceType::Texture:
						ApplyTextureToSelectedEntity(node);
						break;
					case ResourceType::Shader:
						ITR_INFO("Open shader: {}", node->info.path);
						break;
					default:
						ITR_INFO("Open file: {}", node->info.path);
						break;
					}
				}
			}

			// right-click context menu
			if (ImGui::BeginPopupContextItem()) {
				if (ImGui::MenuItem("Open")) {
					if (node->info.type == ResourceType::Model) CreateModelEntity(node);
					else if (node->info.type == ResourceType::Texture) ApplyTextureToSelectedEntity(node);
				}
				if (ImGui::MenuItem("Rename")) {
					m_RenameNode = node;
					std::strncpy(m_RenameBuffer, node->info.name.c_str(), sizeof(m_RenameBuffer) - 1);
					m_RenameBuffer[sizeof(m_RenameBuffer) - 1] = '\0';
					ImGui::OpenPopup("RenamePopup");
				}
				if (ImGui::MenuItem("Delete")) {
					std::error_code ec;
					std::filesystem::remove_all(std::filesystem::path(node->info.path), ec);
					if (ec) {
						ITR_ERROR("Failed to delete '{}': {}", node->info.path, ec.message());
					}
					else {
						ITR_INFO("Deleted '{}'", node->info.path);
					}
					resourceManager.RefreshFileTree();
					ImGui::EndPopup();
					if (nodeOpen) ImGui::TreePop();
					return;
				}
				if (ImGui::MenuItem("Show In Explorer")) {
#ifdef _WIN32
					std::string cmd = std::string("explorer /select,\"") + node->info.path + "\"";
					system(cmd.c_str());
#endif
				}
				ImGui::EndPopup();
			}

			// Inline rename popup (modal)
			if (m_RenameNode == node) {
				if (ImGui::BeginPopupModal("RenamePopup", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
					ImGui::Text("Rename to:");
					ImGui::InputText("##rename", m_RenameBuffer, sizeof(m_RenameBuffer));
					if (ImGui::Button("OK", ImVec2(120, 0))) {
						std::filesystem::path oldPath(node->info.path);
						std::filesystem::path newPath = oldPath.parent_path() / std::string(m_RenameBuffer);
						std::error_code ec;
						std::filesystem::rename(oldPath, newPath, ec);
						if (ec) {
							ITR_ERROR("Rename failed: {}", ec.message());
						}
						else {
							ITR_INFO("Renamed '{}' to '{}'", oldPath.string(), newPath.string());
						}
						m_RenameNode = nullptr;
						resourceManager.RefreshFileTree();
						ImGui::CloseCurrentPopup();
					}
					ImGui::SameLine();
					if (ImGui::Button("Cancel", ImVec2(120, 0))) {
						m_RenameNode = nullptr;
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}
			}

			// recursion
			if (nodeOpen) {
				for (const auto& child : node->children) {
					if (!m_ShowHiddenFiles && !child->info.name.empty() && child->info.name[0] == '.') continue;
					drawNode(child);
				}
				ImGui::TreePop();
			}
			};

		// Draw starting from root's children (skip drawing root as a single node)
		for (const auto& child : fileTree->children) {
			drawNode(child);
		}

		ImGui::EndChild();

		// Right column: details / preview
		ImGui::NextColumn();
		ImGui::BeginChild("ResourceDetails", ImVec2(0, 0), true);
		if (m_SelectedResourceNode) {
			ImGui::Text("Name: %s", m_SelectedResourceNode->info.name.c_str());
			ImGui::Text("Path: %s", m_SelectedResourceNode->info.path.c_str());
			ImGui::Text("Type: %d", (int)m_SelectedResourceNode->info.type);

			// file modified time (portable conversion)
			std::error_code ec;
			if (std::filesystem::exists(m_SelectedResourceNode->info.path, ec) && !ec) {
				auto fsize = std::filesystem::file_size(m_SelectedResourceNode->info.path, ec);
				auto ftime = std::filesystem::last_write_time(m_SelectedResourceNode->info.path, ec);
				if (!ec) {
					using namespace std::chrono;
					auto sctp = time_point_cast<system_clock::duration>(
						ftime - std::filesystem::file_time_type::clock::now()
						+ system_clock::now()
					);
					std::time_t cftime = system_clock::to_time_t(sctp);
					ImGui::Text("Size: %lld bytes", (long long)fsize);
					ImGui::Text("Modified: %s", std::asctime(std::localtime(&cftime)));
				}
			}

			ImGui::Separator();
			if (ImGui::Button("Open with system")) {
#ifdef _WIN32
				ShellExecuteA(NULL, "open", m_SelectedResourceNode->info.path.c_str(), NULL, NULL, SW_SHOW);
#endif
			}
			ImGui::SameLine();
			if (ImGui::Button("Refresh")) {
				resourceManager.RefreshFileTree();
			}
		}
		else {
			ImGui::Text("No resource selected");
		}
		ImGui::EndChild();

		ImGui::Columns(1);
		ImGui::End();
	}


	void ImGuiLayer::DrawFileTreeNode(std::shared_ptr<ResourceFileNode> node) {
		// kept for compatibility with other code that might call this method directly
		if (!node) return;

		// default simple delegator using new UI
		// (you can keep this or remove if not used elsewhere)
		ImGui::Text("%s", node->info.name.c_str());
	}


	void ImGuiLayer::HandleResourceDragDrop(std::shared_ptr<ResourceFileNode> node) {
		if (!node) return;

		switch (node->info.type) {
		case ResourceType::Model:
			ImGui::SetDragDropPayload("RESOURCE_MODEL", &node, sizeof(std::shared_ptr<ResourceFileNode>));
			ImGui::Text("Model: %s", node->info.name.c_str());
			break;
		case ResourceType::Texture:
			ImGui::SetDragDropPayload("RESOURCE_TEXTURE", &node, sizeof(std::shared_ptr<ResourceFileNode>));
			ImGui::Text("Texture: %s", node->info.name.c_str());
			break;
		default:
			break;
		}
	}

	void ImGuiLayer::CreateModelEntity(std::shared_ptr<ResourceFileNode> modelNode) {
		if (!m_SceneManager || !modelNode) return;

		auto* activeScene = m_SceneManager->GetActiveScene();
		if (!activeScene) return;

		// 加载模型
		auto model = ResourceManager::Get().LoadModelFromNode(modelNode);
		if (!model) {
			ITR_ERROR("Failed to load model: {}", modelNode->info.path);
			return;
		}

		// 创建实体
		auto entity = activeScene->CreateEntity();
		auto& reg = activeScene->GetECS().GetRegistry();

		// 设置组件
		reg.emplace<TagComponent>(entity, modelNode->info.name);
		reg.emplace<TransformComponent>(entity);
		reg.emplace<ModelComponent>(entity, model);
		reg.emplace<MaterialComponent>(entity, m_DefaultMaterial);

		RefreshEntityList();
		ITR_INFO("Created model entity: {}", modelNode->info.name);
	}

	void ImGuiLayer::ApplyTextureToSelectedEntity(std::shared_ptr<ResourceFileNode> textureNode) {
		if (m_SelectedEntity == entt::null || !textureNode) return;

		auto* activeScene = m_SceneManager ? m_SceneManager->GetActiveScene() : nullptr;
		if (!activeScene) return;

		auto& reg = activeScene->GetECS().GetRegistry();

		// 加载纹理
		auto texture = ResourceManager::Get().LoadTextureFromNode(textureNode);
		if (!texture) {
			ITR_ERROR("Failed to load texture: {}", textureNode->info.path);
			return;
		}

		// 检查实体是否有材质组件
		if (reg.any_of<MaterialComponent>(m_SelectedEntity)) {
			auto& materialComp = reg.get<MaterialComponent>(m_SelectedEntity);
			if (materialComp.material) {
				// 设置漫反射纹理
				materialComp.material->SetDiffuse(texture);
				ITR_INFO("Applied texture to selected entity: {}", textureNode->info.name);
			}
		}
	}


	void ImGuiLayer::UpdateSelectedEntityTransform()
	{
		if (!m_SceneManager || m_SelectedEntity == entt::null) return;
		auto* activeScene = m_SceneManager->GetActiveScene();
		auto& reg = activeScene->GetECS().GetRegistry();
		if (!reg.valid(m_SelectedEntity) || !reg.any_of<TransformComponent>(m_SelectedEntity)) return;

		auto& t = reg.get<TransformComponent>(m_SelectedEntity);
		t.transform.position = m_TransformEditor.position;
		t.transform.rotation = m_TransformEditor.rotation;
		t.transform.scale = m_TransformEditor.scale;

		if (reg.any_of<LightComponent>(m_SelectedEntity))
		{
			auto& light = reg.get<LightComponent>(m_SelectedEntity);
			if (light.Type == LightType::Directional) {
				glm::vec3 worldDir = t.transform.rotation * light.Direction;
				ITR_INFO("Directional Light direction updated: ({:.2f},{:.2f},{:.2f})", worldDir.x, worldDir.y, worldDir.z);
			}
		}
	}

	void ImGuiLayer::SyncTransformEditor()
	{
		if (!m_SceneManager || m_SelectedEntity == entt::null) return;
		auto* activeScene = m_SceneManager->GetActiveScene();
		auto& reg = activeScene->GetECS().GetRegistry();
		if (!reg.valid(m_SelectedEntity) || !reg.any_of<TransformComponent>(m_SelectedEntity)) return;

		auto& t = reg.get<TransformComponent>(m_SelectedEntity);
		m_TransformEditor.position = t.transform.position;
		m_TransformEditor.rotation = t.transform.rotation;
		m_TransformEditor.scale = t.transform.scale;
		m_EulerAngles = glm::degrees(glm::eulerAngles(t.transform.rotation));
	}

	// -------------------------------------------------------------------------
	// Event dispatchers
	// -------------------------------------------------------------------------
	void ImGuiLayer::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FN(ImGuiLayer::OnMouseButtonPressedEvent));
		dispatcher.Dispatch<MouseButtonReleasedEvent>(BIND_EVENT_FN(ImGuiLayer::OnMouseButtonReleasedEvent));
		dispatcher.Dispatch<MouseMovedEvent>(BIND_EVENT_FN(ImGuiLayer::OnMouseMovedEvent));
		dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FN(ImGuiLayer::OnMouseScrolledEvent));
		dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(ImGuiLayer::OnKeyPressedEvent));
		dispatcher.Dispatch<KeyReleasedEvent>(BIND_EVENT_FN(ImGuiLayer::OnKeyReleasedEvent));
		dispatcher.Dispatch<KeyTypedEvent>(BIND_EVENT_FN(ImGuiLayer::OnKeyTypedEvent));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(ImGuiLayer::OnWindowResizedEvent));
	}

	bool ImGuiLayer::ShouldBlockEvent() const
	{
		ImGuiIO& io = ImGui::GetIO();
		if (io.WantCaptureMouse && !m_ViewportHovered) return true;
		if (m_IsUsingGizmo) return true;
		return false;
	}

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
		io.MouseWheel += e.GetYOffset();
		return ShouldBlockEvent();
	}
	bool ImGuiLayer::OnKeyPressedEvent(KeyPressedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeysDown[e.GetKeyCode()] = true;
		io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
		io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
		io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
		io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];

		// Gizmo hotkeys（仅在视口聚焦）
		if (m_ViewportFocused && !io.WantCaptureKeyboard)
		{
			switch (e.GetKeyCode())
			{
			case GLFW_KEY_W: m_GizmoOperation = ImGuizmo::TRANSLATE; break;
			case GLFW_KEY_E: m_GizmoOperation = ImGuizmo::ROTATE; break;
			case GLFW_KEY_R: m_GizmoOperation = ImGuizmo::SCALE; break;
			case GLFW_KEY_T: m_GizmoMode = (m_GizmoMode == ImGuizmo::LOCAL) ? ImGuizmo::WORLD : ImGuizmo::LOCAL; break;
			}
		}

		return ShouldBlockEvent();
	}
	bool ImGuiLayer::OnKeyReleasedEvent(KeyReleasedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeysDown[e.GetKeyCode()] = false;
		return ShouldBlockEvent();
	}
	bool ImGuiLayer::OnKeyTypedEvent(KeyTypedEvent& /*e*/)
	{
		return ShouldBlockEvent();
	}
	bool ImGuiLayer::OnWindowResizedEvent(WindowResizeEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(e.GetWidth(), e.GetHeight());
		io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
		// 保持返回值以便其他层接收 resize
		return ShouldBlockEvent();
	}

} // namespace Intro
