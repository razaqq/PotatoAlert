// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Core/Preprocessor.hpp"

#include "ReplayParser/Entity.hpp"
#include "ReplayParser/Result.hpp"

#include <filesystem>
#include <vector>


namespace PotatoAlert::ReplayParser {

PA_API ReplayResult<std::vector<EntitySpec>> ParseScripts(const std::filesystem::path& scriptsPath);

}  // namespace PotatoAlert::ReplayParser
