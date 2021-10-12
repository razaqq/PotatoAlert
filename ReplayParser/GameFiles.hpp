// Copyright 2021 <github.com/razaqq>
#pragma once

#include "ByteUtil.hpp"
#include "Math.hpp"

#include <optional>
#include <span>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>


using namespace PotatoAlert;
using ReplayParser::Take;
using ReplayParser::TakeInto;
using ReplayParser::TakeString;
using ReplayParser::TakeWString;

typedef std::span<std::byte> Blob;
typedef std::variant<uint8_t, uint16_t, uint32_t, uint64_t, int8_t, int16_t, int32_t, int64_t, float, double, Vec2, Vec3, std::string, std::wstring, Blob> GameValue;

enum class PrimitiveType
{
	Uint8,
	Uint16,
	Uint32,
	Uint64,
	Int8,
	Int16,
	Int32,
	Int64,
	Float32,
	Float64,
	Vector2,
	Vector3,
	String,
	UnicodeString,
	Blob,
};

inline std::optional<GameValue> Parse(PrimitiveType type, std::span<std::byte>& data)
{
	switch (type)
	{
		case PrimitiveType::Uint8:
			uint8_t v1;
			if (TakeInto(data, v1))
			{
				return v1;
			}
			break;
		case PrimitiveType::Uint16:
			uint16_t v2;
			if (TakeInto(data, v2))
			{
				return v2;
			}
			break;
		case PrimitiveType::Uint32:
			uint32_t v3;
			if (TakeInto(data, v3))
			{
				return v3;
			}
			break;
		case PrimitiveType::Uint64:
			uint64_t v4;
			if (TakeInto(data, v4))
			{
				return v4;
			}
			break;
		case PrimitiveType::Int8:
			int8_t v5;
			if (TakeInto(data, v5))
			{
				return v5;
			}
			break;
		case PrimitiveType::Int16:
			uint16_t v6;
			if (TakeInto(data, v6))
			{
				return v6;
			}
			break;
		case PrimitiveType::Int32:
			uint32_t v7;
			if (TakeInto(data, v7))
			{
				return v7;
			}
			break;
		case PrimitiveType::Int64:
			uint64_t v8;
			if (TakeInto(data, v8))
			{
				return v8;
			}
			break;
		case PrimitiveType::Float32:
			float v9;
			if (TakeInto(data, v9))
			{
				return v9;
			}
			break;
		case PrimitiveType::Float64:
			double v10;
			if (TakeInto(data, v10))
			{
				return v10;
			}
			break;
		case PrimitiveType::Vector2:
			Vec2 v11;
			if (TakeInto(data, v11))
			{
				return v11;
			}
			break;
		case PrimitiveType::Vector3:
			Vec3 v12;
			if (TakeInto(data, v12))
			{
				return v12;
			}
			break;
		case PrimitiveType::String:
			uint8_t size;
			if (!TakeInto(data, size))
			{
				break;
			}
		
			if (size == std::numeric_limits<uint8_t>::max())
			{
				uint16_t stringSize;
				if (!TakeInto(data, stringSize))
				{
					break;
				}
				bool unknown;
				if (!TakeInto(data, unknown))
				{
					break;
				}
				std::string str;
				if (TakeString(data, str, stringSize))
				{
					return str;
				}
			}
			else
			{
				std::string str;
				if (TakeString(data, str, size))
				{
					return str;
				}
			}
			break;
		case PrimitiveType::UnicodeString:
			uint8_t size2;
			if (!TakeInto(data, size2))
			{
				break;
			}

			if (size2 == std::numeric_limits<uint8_t>::max())
			{
				uint16_t stringSize;
				if (!TakeInto(data, stringSize))
				{
					break;
				}
				bool unknown;
				if (!TakeInto(data, unknown))
				{
					break;
				}
				std::string str;
				if (TakeString(data, str, stringSize))
				{
					return str;
				}
			}
			else
			{
				std::wstring str;
				if (TakeWString(data, str, size2))
				{
					return str;
				}
			}
			break;
		case PrimitiveType::Blob:
			uint8_t size3;
			if (!TakeInto(data, size3))
			{
				break;
			}

			if (size3 == std::numeric_limits<uint8_t>::max())
			{
				uint16_t blobSize;
				if (!TakeInto(data, blobSize))
				{
					break;
				}
				bool unknown;
				if (!TakeInto(data, unknown))
				{
					break;
				}
				if (data.size() >= blobSize)
				{
					return Take(data, blobSize);
				}
			}
			else
			{
				if (data.size() >= size3)
				{
					return Take(data, size3);
				}
			}
			break;
	}
	return {};
}

enum class ArgType
{
	Primitive,
	Array,
	FixedDict,
	Tuple,
};

inline GameValue Parse(ArgType type, std::span<std::byte>& data)
{
}

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

inline Flag Parse(const std::string& str)
{
	switch (str)
	{
		case "ALL_CLIENTS":
			return Flag::AllClients;
		case "CELL_PUBLIC_AND_OWN":
			return Flag::CellPublicAndOwn;
		case "OWN_CLIENT":
			return Flag::OwnClient;
		case "BASE_AND_CLIENT":
			return Flag::BaseAndClient;
		case "BASE":
			return Flag::Base;
		case "CELL_PRIVATE":
			return Flag::CellPrivate;
		case "CELL_PUBLIC":
			return Flag::CellPublic;
		default:
			return Flag::Unknown;
	}
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
	std::vector<ArgType> args;

	size_t SortSize()
	{
		/*
		let size = self
            .args
            .iter()
            .map(|arg| arg.sort_size())
            .fold(0, |a, b| a + b);
        if size >= 0xffff {
            0xffff + self.variable_length_header_size
        } else {
            size + self.variable_length_header_size
        }
		 */
	}
};

struct DefFile
{
	std::vector<Method> baseMethods;
	std::vector<Method> cellMethods;
	std::vector<Method> clientMethods;
	std::vector<Property> properties;
	std::vector<std::string> implements;
};

struct EntitySpec
{
	std::string name;
	std::vector<Method> baseMethods;
	std::vector<Method> cellMethods;
	std::vector<Method> clientMethods;
	std::vector<Property> properties;
	std::vector<Property> internalProperties;
};

std::vector<Property> ParseProperties()
{
	
}

void PrintValue(auto&& arg)
{
	using T = std::decay_t<decltype(arg)>;

	if constexpr (std::is_same_v<T, std::string>)
	{
		printf("string: %s\n", arg.c_str());
	}
	else if constexpr (std::is_same_v<T, int>)
	{
		printf("integer: %d\n", arg);
	}
	else if constexpr (std::is_same_v<T, bool>)
	{
		printf("bool: %d\n", arg);
	}
}

GameValue ParseValue()
{

}
