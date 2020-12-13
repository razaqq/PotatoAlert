// Copyright 2020 <github.com/razaqq>

#include "StringTable.hpp"
#include "Config.h"
#include <QString>


QString PotatoAlert::GetString(Keys key)
{
    auto lang = PotatoConfig().get<int>("language");
    return QString::fromStdString(std::string(Strings[lang][(int)(Keys)key]));
}
