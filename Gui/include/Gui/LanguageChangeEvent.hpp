// Copyright 2022 <github.com/razaqq>
#pragma once

#include <QEvent>


namespace PotatoAlert::Gui {

class LanguageChangeEvent : public QEvent
{
public:
	LanguageChangeEvent(int lang) : QEvent(RegisteredType()), m_language(lang) {}
	[[nodiscard]] int GetLanguage() const { return m_language; }
	static Type RegisteredType() { return m_type; }

private:
	const static Type m_type;
	int m_language;
};

}  // namespace PotatoAlert::Gui
