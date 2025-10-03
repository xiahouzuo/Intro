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
#include <glm/gtc/type_ptr.hpp>
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
	 * ��ʼ��ImGui�����ĺͺ��
	 * ע�⣺����OpenGL�����Ĵ��������
	 */
	void ImGuiLayer::OnAttach()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		// ����ImGui���ܣ����ü��̵�����ͣ�������ӿ�
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		// ������ʽΪ��ɫ����
		ImGui::StyleColorsDark();

		// ������ӿ���ʽ
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		// ��ʼ��GLFW��OpenGL���
		GLFWwindow* window = (GLFWwindow*)Application::Get().GetWindow().GetNativeWindow();
		IM_ASSERT(window != nullptr);
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");

		ImGuizmo::SetImGuiContext(ImGui::GetCurrentContext());

		// ��ʼˢ��ʵ���б�
		RefreshEntityList();
	}

	/**
	 * ����ImGui��Դ
	 */
	void ImGuiLayer::OnDetach()
	{
		// �رպ�˲�����������
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	/**
	 * ÿ֡����UI�߼�
	 * ���̣���ʼ֡ -> ����UI -> ��ȾUI -> ������ӿ�
	 */
	void ImGuiLayer::OnUpdate(float deltaTime)
	{
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		// ������ʾ�ߴ�
		io.DisplaySize = ImVec2(app.GetWindow().GetWidth(), app.GetWindow().GetHeight());

		// ����ʱ������
		float time = (float)glfwGetTime();
		io.DeltaTime = m_Time > 0.0f ? (time - m_Time) : (1.0f / 60.0f);
		m_Time = time;

		// ��ʼ��֡
		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();


		// ÿ֡��ʼ�� ImGuizmo
		ImGuizmo::BeginFrame();

		// ����UI�������˳����ƣ�ȷ��������ȷ��
		DrawMenuBar();          // �˵���
		DrawDockSpaceHost();    // ͣ���������������֣�
		DrawViewport();         // ��Ⱦ�ӿ�
		ShowEntityManagerWindow(); // ʵ�������
		ShowSceneControlsWindow(); // ��������

		// ����ѡ��ʵ�壬ͬ�����ݲ���ʾ�����
		if (m_SelectedEntity != entt::null)
		{
			SyncTransformEditor();
			ShowEntityInspectorWindow();
			RenderGizmo();
		}

		// ��ʾģ�͵��봰�ڣ���������
		if (m_ShowImportWindow)
		{
			ShowImportModelWindow();
		}

		// ������Ⱦ ImGui��ԭ�У�
		ImGui::Render();

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// ������ӿڣ�������ƽ̨����ͬ����
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}

	/**
	 * ����ȫ��ͣ������
	 * ���ã���Ϊ����UI���ڵĸ�������֧�ִ�����קͣ��
	 */
	void ImGuiLayer::DrawDockSpaceHost()
	{
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
		ImGuiViewport* viewport = ImGui::GetMainViewport();

		// ��ȡ�˵����߶ȣ����á��Ȼ��Ʋ˵�������ǰ�ᣬȡ��ǰ���ڣ��˵������ĸ߶�
// �ֶ�ָ���˵����߶ȣ�����20���أ��ɸ���ʵ�����������
		float menuBarHeight = 20.0f;

		ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + menuBarHeight));
		ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, viewport->Size.y - menuBarHeight));

		// 1. ����DockSpace����λ�ã�Y��ƫ�Ʋ˵����߶�
		ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + menuBarHeight));

		// 2. ����DockSpace�����ߴ磺�߶� = �ӿڸ߶� - �˵����߶�
		ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, viewport->Size.y - menuBarHeight));

		// �����ӿڹ��������ô��ڱ�־�ȣ�ԭ���벻�䣩...
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGuiWindowFlags host_window_flags = ImGuiWindowFlags_NoTitleBar; /* ������־ */

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
	 * ���Ʋ˵���
	 * �������ļ�����������ģ�ͣ�������ʵ�壨�����壩�����
	 */
	void ImGuiLayer::DrawMenuBar()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Import Model..."))
				{
					m_ShowImportWindow = true; // �򿪵��봰��
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
	 * ������Ⱦ�ӿ�
	 * ���ܣ���ʾ������Ⱦ�������RendererLayer��ȡ���������ߴ�仯��������״̬
	 */
	 // DrawViewport (�滻��ԭ����ʵ��)
	void ImGuiLayer::DrawViewport()
	{
		ImGui::Begin("Viewport");

		m_ViewportHovered = ImGui::IsWindowHovered();
		m_ViewportFocused = ImGui::IsWindowFocused();

		Application::Get().SetViewportState(m_ViewportHovered, m_ViewportFocused, m_IsUsingGizmo);

		ImVec2 avail = ImGui::GetContentRegionAvail();
		if (avail.x < 1.0f) avail.x = 1.0f;
		if (avail.y < 1.0f) avail.y = 1.0f;

		// �ߴ�仯��⣨������ԭ�߼���
		if (m_RendererLayer &&
			((uint32_t)avail.x != (uint32_t)m_LastViewportSize.x ||
				(uint32_t)avail.y != (uint32_t)m_LastViewportSize.y))
		{
			m_RendererLayer->ResizeViewport((uint32_t)avail.x, (uint32_t)avail.y);
			m_LastViewportSize = avail;
		}

		// ��ʾ��Ⱦ����
		void* texID = nullptr;
		if (m_RendererLayer)
		{
			GLuint id = m_RendererLayer->GetSceneTextureID();
			texID = (void*)(intptr_t)id;
		}

		if (texID)
		{
			ImGui::Image(texID, avail, ImVec2(0, 1), ImVec2(1, 0));

			// **�ؼ���ʹ�� Image ��ʵ�� rect �������ӿ�ƫ����ߴ�**
			ImVec2 imageMin = ImGui::GetItemRectMin();
			ImVec2 imageMax = ImGui::GetItemRectMax();
			m_ViewportOffset = imageMin;
			m_ViewportSize = ImVec2(imageMax.x - imageMin.x, imageMax.y - imageMin.y);

			// **������ֱ�ӻ��� gizmo����֤ʹ�øô��ڵ� drawlist / rect��**
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
	 * ��ʾʵ�����������
	 * ���ܣ��г�����������ʵ�塢֧��ѡ�����ק
	 */
	void ImGuiLayer::ShowEntityManagerWindow()
	{
		ImGui::Begin("Entity Manager");

		// ˢ��ʵ���б�ť
		if (ImGui::Button("Refresh List"))
		{
			RefreshEntityList();
		}

		ImGui::Separator();

		// ��ȡ��ǰ�����
		auto* activeScene = m_SceneManager ? m_SceneManager->GetActiveScene() : nullptr;
		if (!activeScene)
		{
			ImGui::Text("No active scene");
			ImGui::End();
			return;
		}

		// ��ʾʵ���б�
		auto& registry = activeScene->GetECS().GetRegistry();
		for (auto entity : m_CachedEntities)
		{
			if (!registry.valid(entity)) // ������Чʵ��
				continue;

			// ����ʵ�����ƣ�������ʾ��ǩ��������ʾID��
			std::string name = "Entity " + std::to_string((uint32_t)entity);
			if (registry.any_of<TagComponent>(entity))
			{
				name = registry.get<TagComponent>(entity).Tag;
			}

			// ��ʾʵ���֧��ѡ��
			ImGui::PushID((uint32_t)entity); // ΨһID������UI��ͻ
			if (ImGui::Selectable(name.c_str(), m_SelectedEntity == entity))
			{
				m_SelectedEntity = entity;
				m_SelectedEntityName = name;
			}

			// ֧����קʵ��
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
	 * ��ʾʵ����������
	 * ���ܣ��༭ѡ��ʵ��������Ŀǰ֧��Transform��
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

		// ��ȡ��ǰ������ʵ��ע���
		auto* activeScene = m_SceneManager ? m_SceneManager->GetActiveScene() : nullptr;
		if (!activeScene)
		{
			ImGui::End();
			return;
		}

		auto& registry = activeScene->GetECS().GetRegistry();
		if (!registry.valid(m_SelectedEntity)) // ���ʵ����Ч��
		{
			m_SelectedEntity = entt::null;
			ImGui::End();
			return;
		}

		// ��ʾʵ������
		ImGui::Text("Entity: %s", m_SelectedEntityName.c_str());
		ImGui::Separator();

		// �༭Transform���
		if (registry.any_of<TransformComponent>(m_SelectedEntity))
		{
			if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
			{
				// ��ʾλ�á���ת��ŷ���ǣ������ű༭��
				ImGui::DragFloat3("Position", &m_TransformEditor.position.x, 0.1f);
				ImGui::DragFloat3("Rotation", &m_EulerAngles.x, 1.0f); // �Ƕȵ�λ
				ImGui::DragFloat3("Scale", &m_TransformEditor.scale.x, 0.1f);

				// Ӧ�ñ任��ʵ��
				if (ImGui::Button("Apply Transform"))
				{
					// ŷ����ת��Ԫ�����ڲ��洢����Ԫ��������������
					m_TransformEditor.rotation = glm::quat(glm::radians(m_EulerAngles));
					UpdateSelectedEntityTransform();
				}
			}
		}

		// ����չ����������������Mesh��Light���ı༭�߼�

		ImGui::End();
	}

	/**
	 * ��ʾ�������ƴ���
	 * ���ܣ���������Ŀ��ƣ�����ճ��������泡���ȣ�
	 */
	void ImGuiLayer::ShowSceneControlsWindow()
	{
		ImGui::Begin("Scene Controls");

		// ʾ������ճ�����ť
		if (ImGui::Button("Clear Scene") && m_SceneManager)
		{
			// m_SceneManager->ClearActiveScene(); // ʵ����Ŀ��ʵ������߼�
		}

		ImGui::Separator();

		// ==== Gizmo ���� ====
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

		// ==== �����ʾ ====
		ImGui::TextUnformatted("Shortcuts:");
		ImGui::BulletText("W - Translate");
		ImGui::BulletText("E - Rotate");
		ImGui::BulletText("R - Scale");
		ImGui::BulletText("Ctrl - Snap (move/rotate/scale)");

		ImGui::End();
	}

	/**
	 * ��ʾģ�͵��봰��
	 * ���ܣ�����ģ��·��������
	 */
	void ImGuiLayer::ShowImportModelWindow()
	{
		ImGui::Begin("Import Model", &m_ShowImportWindow);

		ImGui::InputText("Model Path", &m_ModelImportPath);
		ImGui::SameLine();
		if (ImGui::Button("Browse"))
		{
			// ʵ����Ŀ�пɵ����ļ�ѡ��Ի�����ʹ��nfd�⣩
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
	 * ˢ��ʵ���б��ӳ���ͬ����
	 * �߼����ռ����а���TransformComponent��ʵ�壨ȷ���ǿ���Ⱦ/�ɽ�����ʵ�壩
	 */
	void ImGuiLayer::RefreshEntityList()
	{
		m_CachedEntities.clear();
		auto* activeScene = m_SceneManager ? m_SceneManager->GetActiveScene() : nullptr;
		if (!activeScene)
			return;

		auto& registry = activeScene->GetECS().GetRegistry();
		std::unordered_set<entt::entity> entities; // ��setȥ��

		// �ռ�������TransformComponent��ʵ�壨���������
		auto view = registry.view<TransformComponent>();
		for (auto entity : view)
		{
			entities.insert(entity);
		}

		// ת��Ϊvector���ڱ���
		m_CachedEntities.assign(entities.begin(), entities.end());
	}

	/**
	 * ����ģ�Ͳ�����ʵ��
	 * @param modelPath ģ���ļ�·��
	 * @return �Ƿ���ɹ�
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

		RefreshEntityList();
		return true;
	}

	/**
	 * ��������������ʵ��
	 * @param type ���������ͣ������塢����ȣ�
	 */
	void ImGuiLayer::CreatePrimitive(ShapeType type)
	{
		if (!m_SceneManager) return;

		auto* activeScene = m_SceneManager->GetActiveScene();
		if (!activeScene) return;

		// ʵ����Ŀ��ʹ��ShapeGenerator��������
		// ʾ���߼���
		// std::string name;
		// std::shared_ptr<Mesh> mesh;
		// switch(type)
		// {
		//     case ShapeType::Cube:
		//         name = "Cube";
		//         mesh = ShapeGenerator::CreateCube();
		//         break;
		//     // ��������...
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
	 * ���༭���еı任Ӧ�õ�ѡ��ʵ��
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
	 * ͬ��ѡ��ʵ��ı任���༭����ȷ���༭�����������ݣ�
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
			// ͬ��λ�á���ת������
			m_TransformEditor.position = transform.transform.position;
			m_TransformEditor.rotation = transform.transform.rotation;
			m_TransformEditor.scale = transform.transform.scale;
			// ��Ԫ��תŷ���ǣ���ʾ�ã�
			m_EulerAngles = glm::degrees(glm::eulerAngles(transform.transform.rotation));
		}
	}

	// RenderGizmo (�滻��ԭ����ʵ��)
	void ImGuiLayer::RenderGizmo()
	{
		// ����У�飨������
		if (m_SelectedEntity == entt::null) { std::cout << "Gizmo Debug: No entity selected\n"; return; }
		if (!m_RendererLayer) { std::cout << "Gizmo Debug: No renderer layer\n"; return; }
		if (!m_SceneManager) { std::cout << "Gizmo Debug: No scene manager\n"; return; }

		auto* activeScene = m_SceneManager->GetActiveScene();
		if (!activeScene) { std::cout << "Gizmo Debug: No active scene\n"; return; }

		auto& registry = activeScene->GetECS().GetRegistry();
		if (!registry.valid(m_SelectedEntity)) { std::cout << "Gizmo Debug: Invalid entity\n"; m_SelectedEntity = entt::null; return; }
		if (!registry.all_of<TransformComponent>(m_SelectedEntity)) { std::cout << "Gizmo Debug: Entity has no TransformComponent\n"; return; }

		// ��ȡ�������ȷ��������Ⱦ������ʱʹ�õ������
		auto camera = m_RendererLayer->GetCamera();
		glm::mat4 view = camera.GetViewMat();
		glm::mat4 proj = camera.GetProjectionMat();

		// ��ֹ��Ч����
		if (glm::determinant(view) == 0.0f || glm::determinant(proj) == 0.0f) { std::cout << "Gizmo Debug: Invalid camera matrices\n"; return; }

		// **�ǳ���Ҫ��ʹ�õ�ǰ���ڵ� drawlist������ ImGuizmo ���������� viewport ������**
		ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
		ImGuizmo::SetRect(m_ViewportOffset.x, m_ViewportOffset.y, m_ViewportSize.x, m_ViewportSize.y);

		// ��ȡʵ��任������ model ����
		auto& tc = registry.get<TransformComponent>(m_SelectedEntity);
		glm::mat4 model = glm::translate(glm::mat4(1.0f), tc.transform.position) *
			glm::toMat4(tc.transform.rotation) *
			glm::scale(glm::mat4(1.0f), tc.transform.scale);

		// snap �ж���ʹ�� io.KeyCtrl���� ImGui::IsKeyDown ���ɿ���
		ImGuiIO& io = ImGui::GetIO();
		bool useSnap = io.KeyCtrl;
		float snapValues[3] = { 0.5f, 0.5f, 0.5f };
		if (m_GizmoOperation == ImGuizmo::ROTATE) { snapValues[0] = snapValues[1] = snapValues[2] = 15.0f; }
		else if (m_GizmoOperation == ImGuizmo::SCALE) { snapValues[0] = snapValues[1] = snapValues[2] = 0.1f; }

		// ���� ImGuizmo
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

		// ֻ�������ڲ���ʱ�ŷֽⲢд�����
		if (m_IsUsingGizmo)
		{
			float translation[3], rotationEulerDeg[3], scaleArr[3];
			ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(model), translation, rotationEulerDeg, scaleArr);

			glm::vec3 newTranslation(translation[0], translation[1], translation[2]);
			glm::vec3 newRotationDeg(rotationEulerDeg[0], rotationEulerDeg[1], rotationEulerDeg[2]);
			glm::vec3 newScale(scaleArr[0], scaleArr[1], scaleArr[2]);

			glm::vec3 newRotationRad = glm::radians(newRotationDeg);
			glm::quat newQuat = glm::quat(newRotationRad);

			// ����ʵ��任
			tc.transform.position = newTranslation;
			tc.transform.rotation = newQuat;
			tc.transform.scale = newScale;

			// ͬ���༭�����
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

		// ��� ImGui ��Ҫ������꣬������겻���ӿ��ڣ�����ֹ�¼�
		if (io.WantCaptureMouse && !m_ViewportHovered) {
			return true;
		}

		// �������ʹ�� Gizmo����ֹ�¼�
		if (m_IsUsingGizmo) {
			return true;
		}

		return false;
	}


	// �¼�������������ԭ���¼�ϵͳ�߼������޸ķ���ֵ��
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
		io.MouseWheel += e.GetYOffset();  // ����ImGui����״̬


		return ShouldBlockEvent();
	}

	bool ImGuiLayer::OnKeyPressedEvent(KeyPressedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeysDown[e.GetKeyCode()] = true;

		// �������μ�״̬
		io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
		io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
		io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
		io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];

		// Gizmo��ݼ��������ӿھ۽�ʱ��
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
				// �л�����/����ռ�
				m_GizmoMode = (m_GizmoMode == ImGuizmo::LOCAL) ? ImGuizmo::WORLD : ImGuizmo::LOCAL;
				std::cout << "Gizmo: Switch mode to " << (m_GizmoMode == ImGuizmo::LOCAL ? "LOCAL" : "WORLD") << std::endl;
				break;
			}
		}

		// ���̰����¼���ImGui����ʱ��ֹ����
		return io.WantCaptureKeyboard;
	}

	bool ImGuiLayer::OnKeyReleasedEvent(KeyReleasedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeysDown[e.GetKeyCode()] = false;  // ����ImGui����״̬

		// �����ͷ��¼���ImGui����ʱ��ֹ����
		return io.WantCaptureKeyboard;
	}

	bool ImGuiLayer::OnKeyTypedEvent(KeyTypedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddInputCharacter(e.GetKeyCode());  // ���������ַ���ImGui

		// �ַ������¼���ImGui����ʱ��ֹ����
		return io.WantCaptureKeyboard;
	}

	bool ImGuiLayer::OnWindowResizedEvent(WindowResizeEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(e.GetWidth(), e.GetHeight());
		io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);  // ����ImGui���ڳߴ�

		// ���� resize �¼�ͨ����Ҫ���ݸ������㣨��RendererLayer�����ӿڣ������ַ���false
		return io.WantCaptureKeyboard;
	}

	/**
	 * �¼��ַ������¼������������޸�ԭ���¼�ϵͳ��
	 */
	void ImGuiLayer::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);

		// ���¼���������ʹ��ԭ��ϵͳ��BIND_EVENT_FN�꣩
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