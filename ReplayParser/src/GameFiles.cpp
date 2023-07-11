// Copyright 2021 <github.com/razaqq>

#include "Core/Directory.hpp"
#include "Core/File.hpp"
#include "Core/Log.hpp"

#include "ReplayParser/Entity.hpp"
#include "ReplayParser/GameFiles.hpp"

#include <tinyxml2.h>

#include <format>
#include <functional>
#include <optional>
#include <ranges>
#include <string>
#include <vector>


namespace rp = PotatoAlert::ReplayParser;
using PotatoAlert::Core::File;
using PotatoAlert::Core::Version;
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

std::vector<EntitySpec> rp::ParseScripts(const Version& version, std::string_view gameFilePath)
{
	// this is a shit way of doing this, but thanks to wg its also the only way
	const std::string scriptVersion = version.ToString(".", true);

	fs::path versionDir = fs::path(gameFilePath) / scriptVersion / "scripts";
	if (!fs::exists(versionDir))
	{
		LOG_ERROR("Game scripts for version {} not found.", scriptVersion);
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
			std::string entityName = Core::String::Trim(entityElem->Name());
			DefFile defFile = ParseDef((versionDir / "entity_defs" / std::format("{}.def", entityName)).string(), aliases);
			std::vector<DefFile> interfaces;

			ParseInterfaces((versionDir / "entity_defs" / "interfaces"), aliases, defFile, interfaces);
			interfaces.push_back(defFile);

			DefFile merged = MergeDefs(interfaces);

			// std::ranges::stable_sort(merged.Properties, [](const Property& a, const Property& b) -> bool { return TypeSize(a.Type) < TypeSize(b.Type); });
			std::ranges::stable_sort(merged.ClientMethods, [](const Method& a, const Method& b) -> bool { return a.SortSize() < b.SortSize(); });

			std::vector<std::reference_wrapper<const Property>> clientPropertiesInternal;
			std::vector<std::reference_wrapper<const Property>> clientProperties;
			std::vector<std::reference_wrapper<const Property>> cellProperties;
			std::vector<std::reference_wrapper<const Property>> baseProperties;
			for (const Property& prop : merged.Properties)
			{
				if (prop.Flag & (PropertyFlag::AllClients | PropertyFlag::OtherClients | PropertyFlag::OwnClient | PropertyFlag::CellPublicAndOwn | PropertyFlag::BaseAndClient))
				{
					clientProperties.emplace_back(prop);
				}

				if (prop.Flag & (PropertyFlag::AllClients | PropertyFlag::OtherClients | PropertyFlag::OwnClient | PropertyFlag::CellPublicAndOwn))
				{
					clientPropertiesInternal.emplace_back(prop);
				}

				if (prop.Flag & (PropertyFlag::CellPublicAndOwn | PropertyFlag::CellPublic))
				{
					cellProperties.emplace_back(prop);
				}

				if (prop.Flag & PropertyFlag::BaseAndClient)
				{
					baseProperties.emplace_back(prop);
				}
			}

			std::ranges::stable_sort(clientProperties, [](const Property& a, const Property& b) -> bool { return TypeSize(a.Type) < TypeSize(b.Type); });
			specs.emplace_back(EntitySpec{ entityName, std::move(merged.BaseMethods), std::move(merged.CellMethods), std::move(merged.ClientMethods), std::move(merged.Properties), clientProperties, clientPropertiesInternal, cellProperties, baseProperties });
		}
	}
	else
	{
		LOG_ERROR("entities.xml has no entry 'ClientServerEntities'");
		return {};
	}

	return specs;
}
