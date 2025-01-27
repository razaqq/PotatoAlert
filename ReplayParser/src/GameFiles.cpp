// Copyright 2021 <github.com/razaqq>

#include "Core/Directory.hpp"
#include "Core/File.hpp"
#include "Core/Format.hpp"
#include "Core/String.hpp"
#include "Core/Xml.hpp"

#include "ReplayParser/Entity.hpp"
#include "ReplayParser/GameFiles.hpp"
#include "ReplayParser/Result.hpp"

#include <functional>
#include <optional>
#include <ranges>
#include <string>
#include <vector>


namespace rp = PotatoAlert::ReplayParser;
using PotatoAlert::Core::File;
using PotatoAlert::Core::LoadXml;
using PotatoAlert::Core::Version;
using PotatoAlert::Core::XmlResult;
using namespace PotatoAlert::ReplayParser;
using namespace tinyxml2;

static ReplayResult<std::unordered_map<std::string, ArgType>> ParseAliases(const fs::path& path)
{
	XMLDocument doc;
	XmlResult<void> res = LoadXml(doc, path);
	if (!res)
	{
		return PA_REPLAY_ERROR("Failed to open alias.xml ({}): {}.", path, StringWrap(res.error()));
	}

	XMLNode* root = doc.RootElement();
	if (root == nullptr)
	{
		return PA_REPLAY_ERROR("alias.xml is empty.");
	}

	AliasType aliases;

	for (XMLElement* elem = root->FirstChildElement(); elem != nullptr; elem = elem->NextSiblingElement())
	{
		PA_TRY(type, ParseType(elem, aliases));
		aliases.insert({ elem->Name(), type });
	}
	return aliases;
}

ReplayResult<std::vector<EntitySpec>> rp::ParseScripts(const fs::path& scriptsPath)
{
	PA_TRY_OR_ELSE(aliases, ParseAliases(scriptsPath / "entity_defs" / "alias.xml"),
	{
		return PA_REPLAY_ERROR("Failed to parse aliases: {}", error);
	});

	XMLDocument doc;
	const fs::path entitiesPath(scriptsPath / "entities.xml");
	if (!LoadXml(doc, entitiesPath))
	{
		return PA_REPLAY_ERROR("Failed to open entities.xml ({}): {}.", entitiesPath, StringWrap(doc.ErrorStr()));
	}
	
	XMLNode* root = doc.FirstChild();
	if (root == nullptr)
	{
		return PA_REPLAY_ERROR("entities.xml is empty.");
	}

	std::vector<EntitySpec> specs;
	if (XMLElement* clientServerEntries = root->FirstChildElement("ClientServerEntities"))
	{
		for (XMLElement* entityElem = clientServerEntries->FirstChildElement(); entityElem != nullptr; entityElem = entityElem->NextSiblingElement())
		{
			std::string entityName = Core::String::Trim(entityElem->Name());
			PA_TRY(defFile, ParseDef(scriptsPath / "entity_defs" / fmt::format("{}.def", entityName), aliases));
			std::vector<DefFile> interfaces;

			PA_TRYV(ParseInterfaces((scriptsPath / "entity_defs" / "interfaces"), aliases, defFile, interfaces));
			interfaces.push_back(std::move(defFile));
			PA_TRY(merged, MergeDefs(interfaces));

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
		return PA_REPLAY_ERROR("entities.xml has no entry 'ClientServerEntities'");
	}

	return specs;
}
