#pragma once

#include"Event.h"

namespace Intro
{

	class ITR_API KeyEvent : public Event
	{
	public:
		inline int GetKeyCode() const { return m_KeyCode; };

		//EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
	protected:
		KeyEvent(int keycode)  //仅从派生类能创建
			:m_KeyCode(keycode) { }

		int m_KeyCode;
	};

	class ITR_API KeyPressEvent :public KeyEvent
	{
	public:
		KeyPressEvent(int keycode, int repeatCount)
			:KeyEvent(keycode), m_RepeatCount(repeatCount) { }

		inline int GetRepeatCount() const { return m_RepeatCount; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_KeyCode << "(" << m_RepeatCount << "repeats)";
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyPressed)
	private:
		int m_RepeatCount;
	};

	class ITR_API KeyReleasedEvent :public KeyEvent
	{
	public:
		KeyReleasedEvent(int keycode)
			:KeyEvent(keycode) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_KeyCode << std::endl;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyReleased)
	private:
	};

}