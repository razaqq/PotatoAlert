// Copyright 2020 <github.com/razaqq>
#pragma once

#include <string>


namespace PotatoAlert {

enum class Keys
{
#include "StringTableKeys.i"
};

const std::string_view Languages[] = {
#include "StringTableLanguages.i"
};

const std::string_view Strings[][10] = {
#include "StringTableStrings.i"
};

std::string_view GetString(const std::string& lang, Keys key);

}  // namespace PotatoAlert
