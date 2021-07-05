// Copyright 2021 <github.com/razaqq>
#pragma once

#include <string>


namespace PotatoAlert::Process {

bool CreateNewProcess(std::string_view path, std::string_view args, bool elevated);

}  // namespace PotatoAlert::Process
