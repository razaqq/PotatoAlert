// Copyright 2021 <github.com/razaqq>

#include "Entity.hpp"

#include "Log.hpp"
#include "String.hpp"
#include "Types.hpp"

#include <tinyxml2.h>

#include <filesystem>
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
			std::string name = String::Trim(argElem->Name());
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
			const char* name = propElem->Name();
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
		implements.emplace_back(String::Trim(impElem->GetText()));
	}

	return implements;
}

DefFile rp::ParseDef(std::string_view file, const AliasType& aliases)
{
	DefFile defFile;

	XMLDocument doc;
	if (doc.LoadFile(file.data()) != XML_SUCCESS)
	{
		LOG_ERROR("Failed to open entity definition file ({}): {}.", file, doc.ErrorStr());
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
		defFile.baseMethods = ParseMethodList(baseMethodElem, aliases);
	}

	if (XMLElement* cellMethodsElem = root->FirstChildElement("CellMethods"))
	{
		defFile.cellMethods = ParseMethodList(cellMethodsElem, aliases);
	}

	if (XMLElement* clientMethodsElem = root->FirstChildElement("ClientMethods"))
	{
		defFile.clientMethods = ParseMethodList(clientMethodsElem, aliases);
	}

	if (XMLElement* propertiesElem = root->FirstChildElement("Properties"))
	{
		defFile.properties = ParseProperties(propertiesElem, aliases);
	}

	if (XMLElement* implementsElem = root->FirstChildElement("Implements"))
	{
		defFile.implements = ParseImplements(implementsElem);
	}

	return defFile;
}

DefFile rp::MergeDefs(const std::vector<DefFile>& defs)
{
	DefFile defFile;

	for (const DefFile& def : defs)
	{
		defFile.baseMethods.insert(defFile.baseMethods.end(), def.baseMethods.begin(), def.baseMethods.end());
		defFile.cellMethods.insert(defFile.cellMethods.end(), def.cellMethods.begin(), def.cellMethods.end());
		defFile.clientMethods.insert(defFile.clientMethods.end(), def.clientMethods.begin(), def.clientMethods.end());
		defFile.properties.insert(defFile.properties.end(), def.properties.begin(), def.properties.end());

#ifndef NDEBUG
		assert(defFile.implements.empty());
#else
		if (!defFile.implements.empty())
		{
			LOG_ERROR("DefFile implements is not empty");
		}
#endif
	}

	return defFile;
}

void rp::ParseInterfaces(const fs::path& root, const AliasType& aliases, const DefFile& def, std::vector<DefFile>& out)
{
	for (const std::string& imp : def.implements)
	{
		out.emplace_back(ParseDef((root / std::format("{}.def", imp)).string(), aliases));
		ParseInterfaces(root, aliases, out.back(), out);
	}
}
