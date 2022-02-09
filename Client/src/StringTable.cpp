// Copyright 2020 <github.com/razaqq>

#include "Client/Config.hpp"
#include "Client/StringTable.hpp"

#include <QString>


QString PotatoAlert::Core::StringTable::GetString(Keys key)
{
	auto lang = PotatoConfig().Get<ConfigKey::Language>();
	// TODO: this is pretty terrible, make this std::unordered_map or smth
	return QString::fromUtf8(Strings[lang][static_cast<int>(key)].data());
}
