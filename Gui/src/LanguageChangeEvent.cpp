// Copyright 2022 <github.com/razaqq>

#include "Gui/LanguageChangeEvent.hpp"


using PotatoAlert::Gui::LanguageChangeEvent;

const QEvent::Type LanguageChangeEvent::m_type = static_cast<Type>(registerEventType(User));
