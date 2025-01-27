// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Core/Preprocessor.hpp"
#include "Core/Version.hpp"

#include "ReplayParser/Entity.hpp"
#include "ReplayParser/Result.hpp"

#include <filesystem>
#include <functional>
#include <string>
#include <vector>


namespace PotatoAlert::ReplayParser {

struct EntitySpec
{
	std::string Name;
	std::vector<Method> BaseMethods;
	std::vector<Method> CellMethods;
	std::vector<Method> ClientMethods;
	std::vector<Property> AllProperties;
	std::vector<std::reference_wrapper<const Property>> ClientProperties;
	std::vector<std::reference_wrapper<const Property>> ClientPropertiesInternal;
	std::vector<std::reference_wrapper<const Property>> CellProperties;
	std::vector<std::reference_wrapper<const Property>> BaseProperties;
};

PA_API ReplayResult<std::vector<EntitySpec>> ParseScripts(Core::Version version, const fs::path& gameFilePath);

}  // namespace PotatoAlert::ReplayParser
