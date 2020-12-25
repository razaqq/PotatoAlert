// Copyright 2020 <github.com/razaqq>

#include "StringTable.hpp"
#include "Config.hpp"
#include <QString>


QString PotatoAlert::GetString(StringKeys key)
{
    auto lang = PotatoConfig().get<int>("language");
    return QString::fromUtf8(Strings[lang][key].data());
}
