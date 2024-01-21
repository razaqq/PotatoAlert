// Copyright 2022 <github.com/razaqq>

#include "Gui/Events.hpp"


using PotatoAlert::Gui::FontScalingChangeEvent;
using PotatoAlert::Gui::LanguageChangeEvent;

const QEvent::Type LanguageChangeEvent::m_type = static_cast<Type>(registerEventType(User));
const QEvent::Type FontScalingChangeEvent::m_type = static_cast<Type>(registerEventType(User));
