// Copyright 2022 <github.com/razaqq>
#pragma once

#include <QEvent>


namespace PotatoAlert::Gui {

class LanguageChangeEvent : public QEvent
{
public:
	explicit LanguageChangeEvent(int lang) : QEvent(RegisteredType()), m_language(lang) {}
	[[nodiscard]] int GetLanguage() const { return m_language; }
	static Type RegisteredType() { return m_type; }

private:
	const static Type m_type;
	int m_language;
};

class FontScalingChangeEvent : public QEvent
{
public:
	explicit FontScalingChangeEvent(float scaling) : QEvent(RegisteredType()), m_scaling(scaling) {}
	[[nodiscard]] float GetScaling() const { return m_scaling; }
	static Type RegisteredType() { return m_type; }

private:
	const static Type m_type;
	float m_scaling;
};

}  // namespace PotatoAlert::Gui
