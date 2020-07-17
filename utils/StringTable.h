// Copyright 2020 <github.com/razaqq>
#pragma once

#include <string>
#include <QString>


namespace PotatoAlert {

enum class Keys
{
#include "StringTableKeys.i"
};

const std::string_view Languages[] = {
#include "StringTableLanguages.i"
};

const std::string_view Strings[][100] = {  // TODO: size of array
#include "StringTableStrings.i"
};

QString GetString(Keys key);

}  // namespace PotatoAlert
