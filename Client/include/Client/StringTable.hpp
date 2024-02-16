// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QString>

#include <string>


namespace PotatoAlert::Client::StringTable {

enum class StringTableKey
{
#include "StringTableKeys.i"
};

static constexpr std::string_view Languages[] =
{
#include "StringTableLanguages.i"
};

static constexpr std::string_view Strings[][200] = // TODO: size of array
{
#include "StringTableStrings.i"
};

inline QString GetString(int language, StringTableKey key)
{
	assert((size_t)language <= Languages->size());
	// TODO: this is pretty terrible, make this std::unordered_map or smth
	return QString::fromUtf8(Strings[language][static_cast<int>(key)].data());
}

inline constexpr std::string_view GetStringView(int language, StringTableKey key)
{
	assert((size_t)language <= Languages->size());
	// TODO: this is pretty terrible, make this std::unordered_map or smth
	return Strings[language][static_cast<int>(key)].data();
}

}  // namespace PotatoAlert::Client::StringTable
