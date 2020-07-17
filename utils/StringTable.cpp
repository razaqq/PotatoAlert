// Copyright 2020 <github.com/razaqq>

#include "StringTable.h"
#include "Config.h"


std::string_view PotatoAlert::GetString(const std::string& lang, Keys key)
{
    int langIndex;

    for (int i = 0; i < Languages->size(); i++)
    {
        if (Languages[i] == lang)
            langIndex = i;
    }

    if (langIndex == -1)
        return "";

    return Strings[langIndex][(int)(Keys)key];
}
