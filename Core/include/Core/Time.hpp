// Copyright 2021 <github.com/razaqq>
#pragma once

#include <string>


namespace PotatoAlert::Core::Time {

std::string GetTimeStamp(std::string_view fmt = "%F_%T");

}  // namespace PotatoAlert::Time
