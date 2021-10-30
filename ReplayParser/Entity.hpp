// Copyright 2021 <github.com/razaqq>
#pragma once

#include "String.hpp"
#include "Types.hpp"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>


namespace fs = std::filesystem;

namespace PotatoAlert::ReplayParser {

enum class Flag
{
	AllClients,
	CellPublicAndOwn,
	OwnClient,
	BaseAndClient,
	Base,
	CellPrivate,
	CellPublic,
	OtherClients,
	Unknown,
};

inline Flag ParseFlag(const std::string& str)
{
	static const std::unordered_map<std::string, Flag> flags{
		{ "ALL_CLIENTS", Flag::AllClients },
		{ "CELL_PUBLIC_AND_OWN", Flag::CellPublicAndOwn },
		{ "OWN_CLIENT", Flag::OwnClient },
		{ "BASE_AND_CLIENT", Flag::BaseAndClient },
		{ "BASE", Flag::Base },
		{ "CELL_PRIVATE", Flag::CellPrivate },
		{ "CELL_PUBLIC", Flag::CellPublic }
	};

	if (const auto& lookup = flags.find(String::Trim(String::ToUpper(str))); lookup != flags.end())
	{
		return lookup->second;
	}

	return Flag::Unknown;
}

struct Property
{
	std::string name;
	ArgType type;
	Flag flag;
};

struct Method
{
	std::string name;
	size_t varLengthHeaderSize;
	std::vector<ArgType> args = {};

	[[nodiscard]] size_t SortSize() const
	{
		size_t size = 0;
		for (const auto& arg : this->args)
		{
			size += TypeSize(arg);
		}

		if (size >= Infinity)
		{
			return Infinity + this->varLengthHeaderSize;
		}
		return size + this->varLengthHeaderSize;
	}
};

struct DefFile
{
	std::vector<Method> baseMethods = {};
	std::vector<Method> cellMethods = {};
	std::vector<Method> clientMethods = {};
	std::vector<Property> properties = {};
	std::vector<std::string> implements = {};
};

DefFile ParseDef(std::string_view file, const AliasType& aliases);
DefFile MergeDefs(const std::vector<DefFile>& defs);
void ParseInterfaces(const fs::path& root, const AliasType& aliases, const DefFile& def, std::vector<DefFile>& out);

}  // namespace PotatoAlert::ReplayParser
