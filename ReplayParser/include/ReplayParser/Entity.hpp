// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Core/String.hpp"

#include "ReplayParser/Types.hpp"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>


namespace fs = std::filesystem;

namespace PotatoAlert::ReplayParser {

enum class Flag
{
	AllClients       = 1 << 0,
	CellPublicAndOwn = 1 << 1,
	OwnClient        = 1 << 2,
	BaseAndClient    = 1 << 3,
	Base             = 1 << 4,
	CellPrivate      = 1 << 5,
	CellPublic       = 1 << 6,
	OtherClients     = 1 << 7,
	Unknown          = 1 << 8,
};

inline constexpr Flag operator|(Flag a, Flag b)
{
	using UT = std::underlying_type_t<Flag>;
	return static_cast<Flag>(static_cast<UT>(a) | static_cast<UT>(b));
}

inline constexpr bool operator&(Flag a, Flag b)
{
	using UT = std::underlying_type_t<Flag>;
	return static_cast<UT>(a) & static_cast<UT>(b);
}

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

	if (const auto& lookup = flags.find(Core::String::Trim(Core::String::ToUpper(str))); lookup != flags.end())
	{
		return lookup->second;
	}

	return Flag::Unknown;
}

struct Property
{
	std::string Name;
	ArgType Type;
	Flag Flag;
};

struct Method
{
	std::string Name;
	size_t VarLengthHeaderSize;
	std::vector<ArgType> Args = {};

	[[nodiscard]] size_t SortSize() const
	{
		size_t size = 0;
		for (const auto& arg : Args)
		{
			size += TypeSize(arg);
		}

		if (size >= Infinity)
		{
			return Infinity + VarLengthHeaderSize;
		}
		return size + VarLengthHeaderSize;
	}
};

struct DefFile
{
	std::vector<Method> BaseMethods = {};
	std::vector<Method> CellMethods = {};
	std::vector<Method> ClientMethods = {};
	std::vector<Property> Properties = {};
	std::vector<std::string> Implements = {};
};

DefFile ParseDef(std::string_view file, const AliasType& aliases);
DefFile MergeDefs(const std::vector<DefFile>& defs);
void ParseInterfaces(const fs::path& root, const AliasType& aliases, const DefFile& def, std::vector<DefFile>& out);

}  // namespace PotatoAlert::ReplayParser
