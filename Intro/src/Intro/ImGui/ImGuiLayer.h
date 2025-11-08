#pragma once
#include "Intro/Layer.h"
#include "imgui.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include "ImGuizmo.h"
#include "Intro/Application.h"
#include "Intro/ECS/SceneManager.h"
#include "Intro/RecourceManager/ResourceFileTree.h"
#include "Intro/Events/ApplicationEvent.h"
#include "Intro/Events/KeyEvent.h"
#include "Intro/Events/MouseEvent.h"
#include "Intro/Renderer/RendererLayer.h"
#include "Intro/ECS/Components.h"
#include "glm/gtc/quaternion.hpp"
#include <vector>
#include <string>
#include <memory>

#include "Intro/ECS/GameObjectManager.h" 

namespace Intro {


	class ITR_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer(SceneManager* sceneManager, RendererLayer* rendererLayer = nullptr);
		~ImGuiLayer() override;

		// Layer lifecycle
		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate(float deltaTime) override;
		void OnEvent(Event& event) override;

		void SetRendererLayer(RendererLayer* layer) { m_RendererLayer = layer; }

	private:
		// Lifecycle helpers
		void InitImGui();
		void ShutdownImGui();
		void BeginFrame();
		void EndFrameAndRender();

		// Draw helpers (单一功能函数)
		void DrawMenuBar();
		void DrawDockSpaceHost();
		void DrawViewport();
		void ShowGameObjectManagerWindow();
		void ShowEntityInspectorWindow();
		void ShowSceneControlsWindow();
		void ShowImportModelWindow();
		void RenderGizmo();
		void HandleRenamePopup();

		// Utilities
		void RefreshGameObjectList();
		bool ImportModel(const std::string& path);
		void CreatePrimitive(ShapeType type);
		void CreateLight(LightType type);
		void ShowRendererSettingsWindow();
		void UpdateSelectedEntityTransform();
		void SyncTransformEditor();
		bool ShouldBlockEvent() const;

		void ShowResourceBrowserWindow();
		void DrawFileTreeNode(std::shared_ptr<ResourceFileNode> node);
		void HandleResourceDragDrop(std::shared_ptr<ResourceFileNode> node);
		void CreateModelEntity(std::shared_ptr<ResourceFileNode> modelNode);
		void ApplyTextureToSelectedEntity(std::shared_ptr<ResourceFileNode> textureNode);


		// Event handlers (原样保留，封装 ImGui IO 状态)
		bool OnMouseButtonPressedEvent(MouseButtonPressedEvent& e);
		bool OnMouseButtonReleasedEvent(MouseButtonReleasedEvent& e);
		bool OnMouseMovedEvent(MouseMovedEvent& e);
		bool OnMouseScrolledEvent(MouseScrolledEvent& e);
		bool OnKeyPressedEvent(KeyPressedEvent& e);
		bool OnKeyReleasedEvent(KeyReleasedEvent& e);
		bool OnKeyTypedEvent(KeyTypedEvent& e);
		bool OnWindowResizedEvent(WindowResizeEvent& e);

	private:
		// dependencies
		RendererLayer* m_RendererLayer = nullptr;
		SceneManager* m_SceneManager = nullptr;

		// runtime state
		float m_Time = 0.0f;

		// viewport
		ImVec2 m_ViewportSize = ImVec2(1280, 720);
		ImVec2 m_LastViewportSize = ImVec2(0, 0);
		ImVec2 m_ViewportOffset = ImVec2(0, 0);
		bool m_ViewportHovered = false;
		bool m_ViewportFocused = false;

		// selection / entities
		GameObject m_SelectedGameObject; // 选中 GameObject（初始为无效对象）
		std::string m_SelectedGameObjectName;
		std::vector<GameObject> m_CachedGameObjects;

		// transform editor (temp copy while editing)
		Transform m_TransformEditor;
		glm::vec3 m_EulerAngles = glm::vec3(0.0f);

		// ImGuizmo state
		ImGuizmo::OPERATION m_GizmoOperation = ImGuizmo::TRANSLATE;
		ImGuizmo::MODE m_GizmoMode = ImGuizmo::WORLD;
		bool m_IsUsingGizmo = false;
		bool m_IsOverGizmo = false;

		// rename modal state
		bool m_IsEditingTag = false;
		GameObject m_EditingGameObject = GameObject(); 
		char m_TagEditBuffer[256] = { 0 };
		bool m_ShouldOpenRenamePopup = false;
		bool m_RenamePopupNeedsFocus = false;

		// model import
		std::string m_ModelImportPath;
		bool m_ShowImportWindow = false;


		// optional default material/shader (可注入)
		std::shared_ptr<Material> m_DefaultMaterial;
		std::shared_ptr<Shader> m_DefaultShader = Application::GetShaderLibrary().Get("UnnamedShader");

		// Resource browser state
		std::string m_ResourceSearch = "";
		std::shared_ptr<ResourceFileNode> m_SelectedResourceNode = nullptr;
		std::shared_ptr<ResourceFileNode> m_RenameNode = nullptr;
		char m_RenameBuffer[256] = { 0 };
		bool m_ShowHiddenFiles = false;
		int m_SortMode = 0; // 0=name, 1=time, 2=type
		float m_ResourcePaneWidth = 320.0f; // 左边树宽度

		bool m_ShowPlayControls = true;

		bool m_ShowResourceBrowser = false;
		bool m_ShowGameObjectManager = true; // 默认显示
		bool m_ShowEntityInspector = true; // 默认显示
		bool m_ShowSceneControls = true; // 默认显示
		bool m_ShowRendererSettings = false; // 默认不显示
		bool m_ShowSkyboxSettings = false;
	};

} // namespace Intro
