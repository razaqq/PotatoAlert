// Copyright 2021 <github.com/razaqq>

#include "Core/Encoding.hpp"
#include "Core/Log.hpp"
#include "Core/String.hpp"
#include "Core/Xml.hpp"

#include "ReplayParser/Entity.hpp"
#include "ReplayParser/Types.hpp"

#include <filesystem>
#include <format>
#include <ranges>
#include <string>
#include <vector>


namespace rp = PotatoAlert::ReplayParser;
using namespace PotatoAlert::ReplayParser;
using namespace PotatoAlert;

static std::vector<Method> ParseMethodList(XMLElement* elem, const AliasType& aliases)
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
				args.push_back(ParseType(argElem, aliases));
			}

			if (name == "Args")
			{
				for (XMLElement* argsElem = argElem->FirstChildElement(); argsElem != nullptr; argsElem = argsElem->NextSiblingElement())
				{
					args.push_back(ParseType(argsElem, aliases));
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

static std::vector<Property> ParseProperties(XMLElement* elem, const AliasType& aliases)
{
	std::vector<Property> properties;

	for (XMLElement* propElem = elem->FirstChildElement(); propElem != nullptr; propElem = propElem->NextSiblingElement())
	{
		XMLElement* flagElem = propElem->FirstChildElement("Flags");
		XMLElement* typeElem = propElem->FirstChildElement("Type");
		if (flagElem && typeElem)
		{
			properties.emplace_back(Property{ propElem->Name(), ParseType(typeElem, aliases), ParseFlag(flagElem->GetText()) });
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

DefFile rp::ParseDef(const fs::path& file, const AliasType& aliases)
{
	DefFile defFile;

	XMLDocument doc;
	Core::XmlResult<void> res = Core::LoadXml(doc, file);
	if (!res)
	{
		LOG_ERROR("Failed to open entity definition file ({}): {}.", Core::PathToUtf8(file).value(), res.error());
		return defFile;
	}

	XMLNode* root = doc.RootElement();
	if (root == nullptr)
	{
		LOG_ERROR("{} is empty.", file);
		return defFile;
	}

	if (XMLElement* baseMethodElem = root->FirstChildElement("BaseMethods"))
	{
		defFile.BaseMethods = ParseMethodList(baseMethodElem, aliases);
	}

	if (XMLElement* cellMethodsElem = root->FirstChildElement("CellMethods"))
	{
		defFile.CellMethods = ParseMethodList(cellMethodsElem, aliases);
	}

	if (XMLElement* clientMethodsElem = root->FirstChildElement("ClientMethods"))
	{
		defFile.ClientMethods = ParseMethodList(clientMethodsElem, aliases);
	}

	if (XMLElement* propertiesElem = root->FirstChildElement("Properties"))
	{
		defFile.Properties = ParseProperties(propertiesElem, aliases);
	}

	if (XMLElement* implementsElem = root->FirstChildElement("Implements"))
	{
		defFile.Implements = ParseImplements(implementsElem);
	}

	return defFile;
}

DefFile rp::MergeDefs(const std::vector<DefFile>& defs)
{
	DefFile defFile;

	for (const DefFile& def : defs)
	{
		defFile.BaseMethods.insert(defFile.BaseMethods.end(), def.BaseMethods.begin(), def.BaseMethods.end());
		defFile.CellMethods.insert(defFile.CellMethods.end(), def.CellMethods.begin(), def.CellMethods.end());
		defFile.ClientMethods.insert(defFile.ClientMethods.end(), def.ClientMethods.begin(), def.ClientMethods.end());
		defFile.Properties.insert(defFile.Properties.end(), def.Properties.begin(), def.Properties.end());

#ifndef NDEBUG
		assert(defFile.Implements.empty());
#else
		if (!defFile.Implements.empty())
		{
			LOG_ERROR("DefFile implements is not empty");
		}
#endif
	}

	return defFile;
}

void rp::ParseInterfaces(const fs::path& root, const AliasType& aliases, const DefFile& def, std::vector<DefFile>& out)
{
	for (const std::string& imp : def.Implements)
	{
		out.emplace_back(ParseDef(root / std::format("{}.def", imp), aliases));
		ParseInterfaces(root, aliases, out.back(), out);
	}
}
