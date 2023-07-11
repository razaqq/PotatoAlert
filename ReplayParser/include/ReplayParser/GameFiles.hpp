// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Core/Version.hpp"

#include "ReplayParser/Entity.hpp"

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

std::vector<EntitySpec> ParseScripts(const Core::Version& version, std::string_view gameFilePath);

}  // namespace PotatoAlert::ReplayParser
