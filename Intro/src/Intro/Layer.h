#pragma once

#include"Intro/Core.h"
#include"Intro/Events/Event.h"

namespace Intro {

	class ITR_API Layer
	{
    public:
        Layer(const std::string& name = "Layer");
        virtual ~Layer();

        virtual void OnAttach() {}
        virtual void OnDetach() {}
        virtual void OnUpdate(float deltaTime) {}
        virtual void OnEvent(Event& event) {}

        const std::string& GetName() const { return m_DebugName; }
    protected:
        std::string m_DebugName;
	};

}