// Copyright 2021 <github.com/razaqq>

#include "GameFiles.hpp"

#include "Directory.hpp"
#include "Entity.hpp"
#include "File.hpp"
#include "Log.hpp"

#include <tinyxml2.h>

#include <format>
#include <optional>
#include <ranges>
#include <string>
#include <vector>


namespace rp = PotatoAlert::ReplayParser;
using PotatoAlert::File;
using namespace PotatoAlert::ReplayParser;
using namespace tinyxml2;

static std::optional<std::unordered_map<std::string, ArgType>> ParseAliases(const std::string& path)
{
	XMLDocument doc;
	if (doc.LoadFile(path.c_str()) != XML_SUCCESS)
	{
		LOG_ERROR("Failed to open alias.xml ({}): {}.", path, doc.ErrorStr());
		return {};
	}

	XMLNode* root = doc.RootElement();
	if (root == nullptr)
	{
		LOG_ERROR("alias.xml is empty.");
		return {};
	}

	AliasType aliases;

	for (XMLElement* elem = root->FirstChildElement(); elem != nullptr; elem = elem->NextSiblingElement())
	{
		aliases.insert({ elem->Name(), ParseType(elem, aliases) });
	}
	return aliases;
}

std::vector<EntitySpec> rp::ParseScripts(const Version& version)
{
	const auto rootPath = GetModuleRootPath();
	if (!rootPath.has_value())
	{
		LOG_ERROR("Failed to get module file name: {}", GetLastError());
		return {};
	}

	fs::path versionDir = fs::path(rootPath.value()).remove_filename() / "ReplayVersions" / version.ToString(".", false) / "scripts";
	if (!fs::exists(versionDir))
	{
		LOG_ERROR("Game files for version {} not found in {}.", version.ToString(".", false), versionDir);
		return {};
	}

	auto aliasResult = ParseAliases((versionDir / "entity_defs" / "alias.xml").string());
	if (!aliasResult)
	{
		LOG_ERROR("Failed to parse aliases");
		return {};
	}
	AliasType aliases = aliasResult.value();

	XMLDocument doc;
	std::string entitiesPath((versionDir / "entities.xml").string());
	if (doc.LoadFile(entitiesPath.c_str()) != XML_SUCCESS)
	{
		LOG_ERROR("Failed to open entities.xml ({}): {}.", entitiesPath, doc.ErrorStr());
		return {};
	}

	XMLNode* root = doc.FirstChild();
	if (root == nullptr)
	{
		LOG_ERROR("entities.xml is empty.");
		return {};
	}

	std::vector<EntitySpec> specs;
	if (XMLElement* clientServerEntries = root->FirstChildElement("ClientServerEntities"))
	{
		for (XMLElement* entityElem = clientServerEntries->FirstChildElement(); entityElem != nullptr; entityElem = entityElem->NextSiblingElement())
		{
			std::string entityName = String::Trim(entityElem->Name());
			DefFile defFile = ParseDef((versionDir / "entity_defs" / std::format("{}.def", entityName)).string(), aliases);
			std::vector<DefFile> interfaces;

			ParseInterfaces((versionDir / "entity_defs" / "interfaces"), aliases, defFile, interfaces);
			interfaces.push_back(defFile);

			DefFile merged = MergeDefs(interfaces);

			std::vector<Property> internalProperties;
			std::vector<Property> properties;
			for (const Property& prop : merged.properties)
			{
				if (prop.flag == Flag::AllClients || prop.flag == Flag::OtherClients || prop.flag == Flag::OwnClient || prop.flag == Flag::CellPublicAndOwn)
				{
					internalProperties.push_back(prop);
				}

				if (prop.flag == Flag::AllClients || prop.flag == Flag::OtherClients || prop.flag == Flag::OwnClient || prop.flag == Flag::CellPublicAndOwn || prop.flag == Flag::BaseAndClient)
				{
					properties.push_back(prop);
				}
			}

			std::ranges::stable_sort(properties, [](const Property& a, const Property& b) -> bool { return TypeSize(a.type) < TypeSize(b.type); });
			std::ranges::stable_sort(merged.clientMethods, [](const Method& a, const Method& b) -> bool { return a.SortSize() < b.SortSize(); });


			specs.emplace_back(EntitySpec{ entityName, std::move(merged.baseMethods), std::move(merged.cellMethods), std::move(merged.clientMethods), properties, internalProperties });
		}
	}
	else
	{
		LOG_ERROR("entities.xml has no entry 'ClientServerEntities'");
		return {};
	}

	return specs;
}
