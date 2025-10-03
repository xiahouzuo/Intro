// ImGuiLayer.h
#pragma once
#include "Intro/Layer.h"
#include "imgui.h"
#include <glm/glm.hpp>
#include "ImGuizmo.h"
#include "Intro/ECS/SceneManager.h"
#include "Intro/Events/ApplicationEvent.h"
#include "Intro/Events/KeyEvent.h"
#include "Intro/Events/MouseEvent.h"
#include "Intro/Renderer/RendererLayer.h"
#include "glm/gtc/quaternion.hpp"

namespace Intro {

	/**
	 * ImGuiLayer 负责编辑器UI的渲染与交互
	 * 职责：初始化ImGui、绘制编辑器界面、处理UI输入、同步场景数据
	 */
	class ITR_API ImGuiLayer : public Layer
	{
	public:
		// 构造函数：注入场景管理器和渲染层依赖
		ImGuiLayer(SceneManager* sceneManager, RendererLayer* rendererLayer = nullptr);
		~ImGuiLayer() override;

		// 层生命周期函数
		void OnAttach() override;   // 初始化ImGui上下文和后端
		void OnDetach() override;   // 清理ImGui资源
		void OnUpdate(float deltaTime) override;  // 每帧更新UI
		void OnEvent(Event& event) override;      // 处理事件（不修改原有事件系统）

		// 设置渲染层（可选，用于后期绑定）
		void SetRendererLayer(RendererLayer* layer) { m_RendererLayer = layer; }
	private:
		// 事件处理函数（保持原有事件系统逻辑，不修改返回值行为）
		bool OnMouseButtonPressedEvent(MouseButtonPressedEvent& e);
		bool OnMouseButtonReleasedEvent(MouseButtonReleasedEvent& e);
		bool OnMouseMovedEvent(MouseMovedEvent& e);
		bool OnMouseScrolledEvent(MouseScrolledEvent& e);
		bool OnKeyPressedEvent(KeyPressedEvent& e);
		bool OnKeyReleasedEvent(KeyReleasedEvent& e);
		bool OnKeyTypedEvent(KeyTypedEvent& e);
		bool OnWindowResizedEvent(WindowResizeEvent& e);


	private:
		bool ShouldBlockEvent();

		// UI绘制函数
		void DrawMenuBar();               // 绘制菜单栏（文件、创建等）
		void DrawDockSpaceHost();         // 创建停靠容器（布局基础）
		void DrawViewport();              // 绘制渲染视口（显示场景渲染结果）
		void ShowEntityManagerWindow();   // 显示实体管理器窗口
		void ShowEntityInspectorWindow(); // 显示实体检查器（编辑组件）
		void ShowSceneControlsWindow();   // 显示场景控制窗口
		void ShowImportModelWindow();     // 显示模型导入窗口

		// 工具函数
		void RefreshEntityList();         // 刷新实体列表（从场景同步）
		bool ImportModel(const std::string& modelPath); // 导入模型
		void CreatePrimitive(ShapeType type); // 创建基础几何体
		void UpdateSelectedEntityTransform(); // 应用变换到选中实体
		void SyncTransformEditor();       // 同步选中实体的变换到编辑器

		//imguizmo
		void RenderGizmo();

	private:
		float m_Time = 0.0f;              // 用于计算帧率等时间相关逻辑

		RendererLayer* m_RendererLayer = nullptr; // 渲染层指针（获取渲染结果）
		SceneManager* m_SceneManager = nullptr;   // 场景管理器（操作场景数据）

		entt::entity m_SelectedEntity = entt::null; // 当前选中的实体
		std::string m_SelectedEntityName;          // 选中实体的名称（显示用）

		std::string m_ModelImportPath;    // 模型导入路径
		bool m_ShowImportWindow = false;  // 是否显示导入窗口

		// 视口状态变量
		ImVec2 m_ViewportSize = ImVec2(1280, 720); // 视口尺寸
		ImVec2 m_LastViewportSize = ImVec2(0, 0);  // 上一帧视口尺寸（用于检测变化）
		bool m_ViewportHovered = false;   // 视口是否被鼠标悬停
		bool m_ViewportFocused = false;   // 视口是否获得焦点
		ImVec2 m_ViewportOffset = ImVec2(0, 0);    // 视口在屏幕上的偏移（用于坐标转换）

		std::vector<entt::entity> m_CachedEntities; // 缓存的实体列表（减少ECS查询次数）
		Transform m_TransformEditor;      // 变换编辑器的临时数据（编辑时不直接修改实体）
		glm::vec3 m_EulerAngles;          // 欧拉角（用于显示旋转，内部仍用四元数计算）

		// ImGuizmo 状态
		ImGuizmo::OPERATION m_GizmoOperation = ImGuizmo::TRANSLATE;
		ImGuizmo::MODE m_GizmoMode = ImGuizmo::WORLD;
		bool m_IsUsingGizmo = false;       // 当前 gizmo 是否被拖动
		bool m_IsOverGizmo = false;        // 鼠标是否悬停在 gizmo 上

		
	};

}