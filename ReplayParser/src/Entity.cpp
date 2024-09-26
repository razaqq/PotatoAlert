// Copyright 2021 <github.com/razaqq>

#include "Core/Format.hpp"
#include "Core/String.hpp"
#include "Core/Xml.hpp"

#include "ReplayParser/Entity.hpp"
#include "ReplayParser/Types.hpp"

#include <filesystem>
#include <ranges>
#include <string>
#include <vector>


namespace rp = PotatoAlert::ReplayParser;
using namespace PotatoAlert::ReplayParser;
using namespace PotatoAlert;

namespace {

static ReplayResult<std::vector<Method>> ParseMethodList(XMLElement* elem, const AliasType& aliases)
{
	std::vector<Method> methods;

	for (XMLElement* methodElem = elem->FirstChildElement(); methodElem != nullptr; methodElem = methodElem->NextSiblingElement())
	{
		std::vector<ArgType> args;
		size_t varLengthHeaderSize = 1;

		for (XMLElement* argElem = methodElem->FirstChildElement(); argElem != nullptr; argElem = argElem->NextSiblingElement())
		{
			std::string name = Core::String::Trim(argElem->Name());
			if (name == "Arg")
			{
				PA_TRY(type, ParseType(argElem, aliases));
				args.push_back(type);
			}

			if (name == "Args")
			{
				for (XMLElement* argsElem = argElem->FirstChildElement(); argsElem != nullptr; argsElem = argsElem->NextSiblingElement())
				{
					PA_TRY(type, ParseType(argsElem, aliases));
					args.push_back(type);
				}
			}

			if (name == "VariableLengthHeaderSize")
			{
				varLengthHeaderSize = argElem->IntText(1);
			}
		}
		methods.emplace_back(Method{ methodElem->Name(), varLengthHeaderSize, args });
	}

	return methods;
}

static ReplayResult<std::vector<Property>> ParseProperties(XMLElement* elem, const AliasType& aliases)
{
	std::vector<Property> properties;

	for (XMLElement* propElem = elem->FirstChildElement(); propElem != nullptr; propElem = propElem->NextSiblingElement())
	{
		XMLElement* flagElem = propElem->FirstChildElement("Flags");
		XMLElement* typeElem = propElem->FirstChildElement("Type");
		if (flagElem && typeElem)
		{
			PA_TRY(type, ParseType(typeElem, aliases));
			properties.emplace_back(Property{ propElem->Name(), type, ParseFlag(flagElem->GetText()) });
		}
	}

	return properties;
}

static std::vector<std::string> ParseImplements(XMLElement* elem)
{
	std::vector<std::string> implements;

	for (XMLElement* impElem = elem->FirstChildElement(); impElem != nullptr; impElem = impElem->NextSiblingElement())
	{
		implements.emplace_back(Core::String::Trim(impElem->GetText()));
	}

	return implements;
}

}

ReplayResult<DefFile> rp::ParseDef(const fs::path& file, const AliasType& aliases)
{
	DefFile defFile;

	XMLDocument doc;
	Core::XmlResult<void> res = Core::LoadXml(doc, file);
	if (!res)
	{
		return PA_REPLAY_ERROR("Failed to open entity definition file ({}): {}.", file, StringWrap(res.error()));
	}

	XMLNode* root = doc.RootElement();
	if (root == nullptr)
	{
		return PA_REPLAY_ERROR("{} is empty.", file);
	}

	if (XMLElement* baseMethodElem = root->FirstChildElement("BaseMethods"))
	{
		PA_TRYA(defFile.BaseMethods, ParseMethodList(baseMethodElem, aliases));
	}

	if (XMLElement* cellMethodsElem = root->FirstChildElement("CellMethods"))
	{
		PA_TRYA(defFile.CellMethods, ParseMethodList(cellMethodsElem, aliases));
	}

	if (XMLElement* clientMethodsElem = root->FirstChildElement("ClientMethods"))
	{
		PA_TRYA(defFile.ClientMethods, ParseMethodList(clientMethodsElem, aliases));
	}

	if (XMLElement* propertiesElem = root->FirstChildElement("Properties"))
	{
		PA_TRYA(defFile.Properties, ParseProperties(propertiesElem, aliases));
	}

	if (XMLElement* implementsElem = root->FirstChildElement("Implements"))
	{
		defFile.Implements = ParseImplements(implementsElem);
	}

	return defFile;
}

ReplayResult<DefFile> rp::MergeDefs(const std::vector<DefFile>& defs)
{
	DefFile defFile;

	for (const DefFile& def : defs)
	{
		defFile.BaseMethods.insert(defFile.BaseMethods.end(), def.BaseMethods.begin(), def.BaseMethods.end());
		defFile.CellMethods.insert(defFile.CellMethods.end(), def.CellMethods.begin(), def.CellMethods.end());
		defFile.ClientMethods.insert(defFile.ClientMethods.end(), def.ClientMethods.begin(), def.ClientMethods.end());
		defFile.Properties.insert(defFile.Properties.end(), def.Properties.begin(), def.Properties.end());

		if (!defFile.Implements.empty())
		{
			return PA_REPLAY_ERROR("DefFile implements is not empty");
		}
	}

	return defFile;
}

ReplayResult<void> rp::ParseInterfaces(const fs::path& root, const AliasType& aliases, const DefFile& def, std::vector<DefFile>& out)
{
	for (const std::string& imp : def.Implements)
	{
		PA_TRY(defFile, ParseDef(root / fmt::format("{}.def", imp), aliases));
		out.emplace_back(std::move(defFile));
		PA_TRYV(ParseInterfaces(root, aliases, out.back(), out));
	}

	return {};
}
