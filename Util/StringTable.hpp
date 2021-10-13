// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QString>

#include <string_view>


namespace PotatoAlert::StringTable {

enum class Keys
{
#include "StringTableKeys.i"
};

static const std::string_view Languages[] =
{
#include "StringTableLanguages.i"
};

static const std::string_view Strings[][200] =
{  // TODO: size of array
#include "StringTableStrings.i"
};

QString GetString(Keys key);

}  // namespace PotatoAlert::StringTable
