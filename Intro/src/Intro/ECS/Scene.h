#pragma once

#include <string>
#include <memory>
#include "ECS.h"
#include "Intro/Core.h"

namespace Intro {

    // Scene ��ʾһ������������ / ������ӵ���Լ��� ECS/registry �ȣ�
    class ITR_API Scene {
    public:
        explicit Scene(std::string name = "Untitled Scene")
            : m_Name(std::move(name)) {
        }

        virtual ~Scene() = default;

        // ���ÿ������ƶ���entt::registry �����ǵ� ECS �в��ɿ���/�ƶ���
        Scene(const Scene&) = delete;
        Scene& operator=(const Scene&) = delete;
        Scene(Scene&&) = delete;
        Scene& operator=(Scene&&) = delete;

        // �������ڹ��ӣ�������Ը�����Щ��������Դ����/ж��/ÿ֡�߼�
        // �������У�ͨ�� Application/SceneManager ���������Щ
        virtual void OnLoad() {}
        virtual void OnUnload() {}
        virtual void OnUpdate(float dt) {}

        // ���� ECS���� const����ϵͳͨ����Ҫд������ view() ����
        ECS& GetECS() { return m_ECS; }
        const ECS& GetECS() const { return m_ECS; }

        // ��� API ת������ѡ��
        Intro::ECS::Entity CreateEntity() { return m_ECS.CreateEntity(); }
        void DestroyEntity(Intro::ECS::Entity e) { m_ECS.DestroyEntity(e); }

        // ����
        const std::string& GetName() const { return m_Name; }
        void SetName(const std::string& name) { m_Name = name; }

        // ����/ͣ�ã�ĳЩ����������ʱ�����µ���������
        void SetActive(bool active) { m_Active = active; }
        bool IsActive() const { return m_Active; }

    protected:
        // ������Է��� m_ECS ������/ע��ϵͳ���������� OnLoad �������ӵĳ�ʼ��
        ECS m_ECS;

    private:
        std::string m_Name;
        bool m_Active = true;
    };

}