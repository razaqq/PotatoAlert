// Copyright 2020 <github.com/razaqq>

#include "StringTable.hpp"

#include "Config.hpp"

#include <QString>


QString PotatoAlert::StringTable::GetString(Keys key)
{
	auto lang = PotatoConfig().Get<int>("language");
	// TODO: this is pretty terrible, make this std::unordered_map or smth
	return QString::fromUtf8(Strings[lang][static_cast<int>(key)].data());
}
