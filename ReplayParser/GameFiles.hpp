// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Entity.hpp"
#include "Version.hpp"

#include <string>
#include <vector>


namespace PotatoAlert::ReplayParser {

struct EntitySpec
{
	std::string name;
	std::vector<Method> baseMethods;
	std::vector<Method> cellMethods;
	std::vector<Method> clientMethods;
	std::vector<Property> properties;
	std::vector<Property> internalProperties;
};

std::vector<EntitySpec> ParseScripts(const Version& version);

}  // namespace PotatoAlert::ReplayParser
