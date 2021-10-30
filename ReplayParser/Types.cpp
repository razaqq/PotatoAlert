// Copyright 2021 <github.com/razaqq>

#include "Types.hpp"

#include "ByteUtil.hpp"
#include "String.hpp"

#include <optional>
#include <string>
#include <unordered_map>

#include <tinyxml2.h>


namespace rp = PotatoAlert::ReplayParser;
using namespace PotatoAlert::ReplayParser;

ArgType rp::ParseType(XMLElement* elem, const AliasType& aliases)
{
	static const std::unordered_map<std::string, BasicType> types{
		{ "UINT8", BasicType::Uint8 },
		{ "UINT16", BasicType::Uint16 },
		{ "UINT32", BasicType::Uint32 },
		{ "UINT64", BasicType::Uint64 },
		{ "INT8", BasicType::Int8 },
		{ "INT16", BasicType::Int16 },
		{ "INT32", BasicType::Int32 },
		{ "INT64", BasicType::Int64 },
		{ "FLOAT32", BasicType::Float32 },
		{ "FLOAT", BasicType::Float32 },
		{ "FLOAT64", BasicType::Float64 },
		{ "STRING", BasicType::String },
		{ "UNICODE_STRING", BasicType::UnicodeString },
		{ "VECTOR2", BasicType::Vector2 },
		{ "VECTOR3", BasicType::Vector3 },
		{ "BLOB", BasicType::Blob },

		{ "USER_TYPE", BasicType::Blob },  // TODO: these 3 can be parsed better
		{ "MAILBOX", BasicType::Blob },
		{ "PYTHON", BasicType::Blob },  // TODO end
	};

	std::string typeName = String::ToUpper(String::Trim(elem->GetText()));
	if (types.contains(typeName))
	{
		return PrimitiveType{ types.at(typeName) };
	}

	if (typeName == "ARRAY")
	{
		ArrayType arr{};
		if (XMLElement* ofElem = elem->FirstChildElement("of"))
		{
			arr.subType = std::make_shared<ArgType>(ParseType(ofElem, aliases));
		}

		if (XMLElement* sizeElem = elem->FirstChildElement("size"))
		{
			size_t size;
			if (String::ParseNumber<size_t>(sizeElem->GetText(), size))
			{
				arr.size = size;
			}
		}

		return arr;
	}

	if (typeName == "FIXED_DICT")
	{
		FixedDictType dict;
		dict.allowNone = elem->BoolAttribute("AllowNone", false);

		if (XMLElement* propElem = elem->FirstChildElement("Properties"))
		{
			for (XMLElement* prop = propElem->FirstChildElement(); prop != nullptr; prop = prop->NextSiblingElement())
			{
				if (XMLElement* typeElem = prop->FirstChildElement("Type"))
				{
					dict.properties.emplace_back(FixedDictProperty{ prop->Name(), std::make_shared<ArgType>(ParseType(typeElem, aliases)) });
				}
			}
		}

		return dict;
	}

	if (typeName == "TUPLE")
	{
		TupleType tuple{};
		if (XMLElement* ofElem = elem->FirstChildElement("of"))
		{
			tuple.subType = std::make_shared<ArgType>(ParseType(ofElem, aliases));
		}

		if (XMLElement* sizeElem = elem->FirstChildElement("size"))
		{
			size_t size;
			if (String::ParseNumber<size_t>(sizeElem->GetText(), size))
			{
				tuple.size = size;
			}
		}
		return tuple;
	}

	if (aliases.contains(typeName))
	{
		return aliases.at(typeName);
	}

	return UnknownType{};
}

size_t rp::TypeSize(const ArgType& type)
{
	return std::visit([](auto&& arg) -> size_t
	{
		using T = std::decay_t<decltype(arg)>;

		if constexpr (std::is_same_v<T, PrimitiveType>)
		{
			return PrimitiveSize(arg.type);
		}

		if constexpr (std::is_same_v<T, ArrayType>)
		{
			if (!arg.size) return Infinity;
			size_t size = TypeSize(*arg.subType);
			if (size == Infinity) return Infinity;
			return size * arg.size.value();
		}

		if constexpr (std::is_same_v<T, FixedDictType>)
		{
			if (arg.allowNone) return Infinity;
			size_t totalSize = 0;
			for (const FixedDictProperty& prop : arg.properties)
			{
				const size_t size = TypeSize(*prop.type);
				if (size == Infinity) return Infinity;
				totalSize += size;
			}
			return totalSize;
		}

		if constexpr (std::is_same_v<T, TupleType>)
		{
			size_t size = TypeSize(*arg.subType);
			if (size == Infinity) return Infinity;
			return size * arg.size;
		}

		return Infinity;
	},
	type);
}

static std::optional<ArgValue> ParsePrimitive(PrimitiveType type, std::span<std::byte>& data)
{
	switch (type.type)
	{
		case BasicType::Uint8:
			uint8_t v1;
			if (TakeInto(data, v1))
			{
				return v1;
			}
			break;
		case BasicType::Uint16:
			uint16_t v2;
			if (TakeInto(data, v2))
			{
				return v2;
			}
			break;
		case BasicType::Uint32:
			uint32_t v3;
			if (TakeInto(data, v3))
			{
				return v3;
			}
			break;
		case BasicType::Uint64:
			uint64_t v4;
			if (TakeInto(data, v4))
			{
				return v4;
			}
			break;
		case BasicType::Int8:
			int8_t v5;
			if (TakeInto(data, v5))
			{
				return v5;
			}
			break;
		case BasicType::Int16:
			uint16_t v6;
			if (TakeInto(data, v6))
			{
				return v6;
			}
			break;
		case BasicType::Int32:
			uint32_t v7;
			if (TakeInto(data, v7))
			{
				return v7;
			}
			break;
		case BasicType::Int64:
			uint64_t v8;
			if (TakeInto(data, v8))
			{
				return v8;
			}
			break;
		case BasicType::Float32:
			float v9;
			if (TakeInto(data, v9))
			{
				return v9;
			}
			break;
		case BasicType::Float64:
			double v10;
			if (TakeInto(data, v10))
			{
				return v10;
			}
			break;
		case BasicType::Vector2:
			Vec2 v11;
			if (TakeInto(data, v11))
			{
				return v11;
			}
			break;
		case BasicType::Vector3:
			Vec3 v12;
			if (TakeInto(data, v12))
			{
				return v12;
			}
			break;
		case BasicType::String:
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
		case BasicType::UnicodeString:
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
		case BasicType::Blob:
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
		default:
			break;
	}
	return {};
}

std::optional<ArgValue> rp::ParseValue(std::span<std::byte>& data, const ArgType& type)
{
	return std::visit([&data](auto&& t) -> ArgValue
	{
		using T = std::decay_t<decltype(t)>;
		if constexpr (std::is_same_v<T, PrimitiveType>)
		{
			return ParsePrimitive(t, data);
		}
		else if constexpr (std::is_same_v<T, ArrayType>)
		{
			std::vector<ArgValue> values;
			uint8_t size = 0;
			if (!t.size)
			{
				if (!TakeInto(data, size))
				{
					return {};
				}
			}
			for (size_t i = 0; i < size; i++)
			{
				values.emplace_back(ParseValue(data, *t.subType));
			}
			return values;
		}
		else if constexpr (std::is_same_v<T, FixedDictType>)
		{
			std::unordered_map<std::string, ArgValue> dict;
			if (t.allowNone)
			{
				uint8_t flag;
				if (!TakeInto(data, flag))
				{
					return {};
				}

				if (flag == 0)
				{
					return dict;
				}
				if (flag != 1)
				{
					return {};  // Unknown fixed dict flag
				}
			}

			for (const FixedDictProperty& property : t.properties)
			{
				dict.emplace(property.name, ParseValue(data, *property.type));
			}

			return dict;
		}
		else if constexpr (std::is_same_v<T, TupleType>)
		{
			// TODO
			return {};
		}
		return {};
	}, type);
}
