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
	 * ImGuiLayer ����༭��UI����Ⱦ�뽻��
	 * ְ�𣺳�ʼ��ImGui�����Ʊ༭�����桢����UI���롢ͬ����������
	 */
	class ITR_API ImGuiLayer : public Layer
	{
	public:
		// ���캯����ע�볡������������Ⱦ������
		ImGuiLayer(SceneManager* sceneManager, RendererLayer* rendererLayer = nullptr);
		~ImGuiLayer() override;

		// ���������ں���
		void OnAttach() override;   // ��ʼ��ImGui�����ĺͺ��
		void OnDetach() override;   // ����ImGui��Դ
		void OnUpdate(float deltaTime) override;  // ÿ֡����UI
		void OnEvent(Event& event) override;      // �����¼������޸�ԭ���¼�ϵͳ��

		// ������Ⱦ�㣨��ѡ�����ں��ڰ󶨣�
		void SetRendererLayer(RendererLayer* layer) { m_RendererLayer = layer; }
	private:
		// �¼�������������ԭ���¼�ϵͳ�߼������޸ķ���ֵ��Ϊ��
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

		// UI���ƺ���
		void DrawMenuBar();               // ���Ʋ˵������ļ��������ȣ�
		void DrawDockSpaceHost();         // ����ͣ�����������ֻ�����
		void DrawViewport();              // ������Ⱦ�ӿڣ���ʾ������Ⱦ�����
		void ShowEntityManagerWindow();   // ��ʾʵ�����������
		void ShowEntityInspectorWindow(); // ��ʾʵ���������༭�����
		void ShowSceneControlsWindow();   // ��ʾ�������ƴ���
		void ShowImportModelWindow();     // ��ʾģ�͵��봰��

		// ���ߺ���
		void RefreshEntityList();         // ˢ��ʵ���б��ӳ���ͬ����
		bool ImportModel(const std::string& modelPath); // ����ģ��
		void CreatePrimitive(ShapeType type); // ��������������
		void UpdateSelectedEntityTransform(); // Ӧ�ñ任��ѡ��ʵ��
		void SyncTransformEditor();       // ͬ��ѡ��ʵ��ı任���༭��

		//imguizmo
		void RenderGizmo();

	private:
		float m_Time = 0.0f;              // ���ڼ���֡�ʵ�ʱ������߼�

		RendererLayer* m_RendererLayer = nullptr; // ��Ⱦ��ָ�루��ȡ��Ⱦ�����
		SceneManager* m_SceneManager = nullptr;   // �����������������������ݣ�

		entt::entity m_SelectedEntity = entt::null; // ��ǰѡ�е�ʵ��
		std::string m_SelectedEntityName;          // ѡ��ʵ������ƣ���ʾ�ã�

		std::string m_ModelImportPath;    // ģ�͵���·��
		bool m_ShowImportWindow = false;  // �Ƿ���ʾ���봰��

		// �ӿ�״̬����
		ImVec2 m_ViewportSize = ImVec2(1280, 720); // �ӿڳߴ�
		ImVec2 m_LastViewportSize = ImVec2(0, 0);  // ��һ֡�ӿڳߴ磨���ڼ��仯��
		bool m_ViewportHovered = false;   // �ӿ��Ƿ������ͣ
		bool m_ViewportFocused = false;   // �ӿ��Ƿ��ý���
		ImVec2 m_ViewportOffset = ImVec2(0, 0);    // �ӿ�����Ļ�ϵ�ƫ�ƣ���������ת����

		std::vector<entt::entity> m_CachedEntities; // �����ʵ���б�����ECS��ѯ������
		Transform m_TransformEditor;      // �任�༭������ʱ���ݣ��༭ʱ��ֱ���޸�ʵ�壩
		glm::vec3 m_EulerAngles;          // ŷ���ǣ�������ʾ��ת���ڲ�������Ԫ�����㣩

		// ImGuizmo ״̬
		ImGuizmo::OPERATION m_GizmoOperation = ImGuizmo::TRANSLATE;
		ImGuizmo::MODE m_GizmoMode = ImGuizmo::WORLD;
		bool m_IsUsingGizmo = false;       // ��ǰ gizmo �Ƿ��϶�
		bool m_IsOverGizmo = false;        // ����Ƿ���ͣ�� gizmo ��

		
	};

}