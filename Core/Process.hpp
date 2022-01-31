// Copyright 2021 <github.com/razaqq>
#pragma once

#include <string>


namespace PotatoAlert::Core {

bool CreateNewProcess(std::string_view path, std::string_view args, bool elevated);

}  // namespace PotatoAlert::Core
