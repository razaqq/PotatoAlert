// Copyright 2020 <github.com/razaqq>
#pragma once

#include <string>
#include <QString>


namespace PotatoAlert {

enum StringKeys
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

QString GetString(StringKeys key);

}  // namespace PotatoAlert
