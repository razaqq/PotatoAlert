// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Core/String.hpp"

#include "ReplayParser/Result.hpp"
#include "ReplayParser/Types.hpp"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>


namespace fs = std::filesystem;

namespace PotatoAlert::ReplayParser {

enum class PropertyFlag
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

inline constexpr PropertyFlag operator|(PropertyFlag a, PropertyFlag b)
{
	using UT = std::underlying_type_t<PropertyFlag>;
	return static_cast<PropertyFlag>(static_cast<UT>(a) | static_cast<UT>(b));
}

inline constexpr bool operator&(PropertyFlag a, PropertyFlag b)
{
	using UT = std::underlying_type_t<PropertyFlag>;
	return static_cast<UT>(a) & static_cast<UT>(b);
}

inline PropertyFlag ParseFlag(const std::string& str)
{
	[[clang::no_destroy]] static const std::unordered_map<std::string, PropertyFlag> flags
	{
		{ "ALL_CLIENTS", PropertyFlag::AllClients },
		{ "CELL_PUBLIC_AND_OWN", PropertyFlag::CellPublicAndOwn },
		{ "OWN_CLIENT", PropertyFlag::OwnClient },
		{ "BASE_AND_CLIENT", PropertyFlag::BaseAndClient },
		{ "BASE", PropertyFlag::Base },
		{ "CELL_PRIVATE", PropertyFlag::CellPrivate },
		{ "CELL_PUBLIC", PropertyFlag::CellPublic }
	};

	if (const auto& lookup = flags.find(Core::String::Trim(Core::String::ToUpper(str))); lookup != flags.end())
	{
		return lookup->second;
	}

	return PropertyFlag::Unknown;
}

struct Property
{
	std::string Name;
	ArgType Type;
	PropertyFlag Flag;
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

ReplayResult<DefFile> ParseDef(const fs::path& file, const AliasType& aliases);
ReplayResult<DefFile> MergeDefs(const std::vector<DefFile>& defs);
ReplayResult<void> ParseInterfaces(const fs::path& root, const AliasType& aliases, const DefFile& def, std::vector<DefFile>& out);

}  // namespace PotatoAlert::ReplayParser
