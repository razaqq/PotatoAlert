// Copyright 2021 <github.com/razaqq>

#include "Core/Bytes.hpp"
#include "Core/Format.hpp"
#include "Core/String.hpp"
#include "Core/Xml.hpp"

#include "ReplayParser/Result.hpp"
#include "ReplayParser/Types.hpp"

#include <memory>
#include <optional>
#include <span>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <variant>


namespace rp = PotatoAlert::ReplayParser;
using namespace PotatoAlert;
using namespace ReplayParser;
using Core::Byte;
using Core::Take;
using Core::TakeInto;
using Core::TakeString;

namespace {

static ReplayResult<ArgValue> ParsePrimitive(PrimitiveType type, std::span<const Byte>& data)
{
	switch (type.Type)
	{
		case BasicType::Uint8:
		{
			uint8_t v;
			if (TakeInto(data, v))
			{
				return v;
			}
			break;
		}
		case BasicType::Uint16:
		{
			uint16_t v;
			if (TakeInto(data, v))
			{
				return v;
			}
			break;
		}
		case BasicType::Uint32:
		{
			uint32_t v;
			if (TakeInto(data, v))
			{
				return v;
			}
			break;
		}
		case BasicType::Uint64:
		{
			uint64_t v;
			if (TakeInto(data, v))
			{
				return v;
			}
			break;
		}
		case BasicType::Int8:
		{
			int8_t v;
			if (TakeInto(data, v))
			{
				return v;
			}
			break;
		}
		case BasicType::Int16:
		{
			int16_t v;
			if (TakeInto(data, v))
			{
				return v;
			}
			break;
		}
		case BasicType::Int32:
		{
			int32_t v;
			if (TakeInto(data, v))
			{
				return v;
			}
			break;
		}
		case BasicType::Int64:
		{
			int64_t v;
			if (TakeInto(data, v))
			{
				return v;
			}
			break;
		}
		case BasicType::Float32:
		{
			float v;
			if (TakeInto(data, v))
			{
				return v;
			}
			break;
		}
		case BasicType::Float64:
		{
			double v;
			if (TakeInto(data, v))
			{
				return v;
			}
			break;
		}
		case BasicType::Vector2:
		{
			Vec2 v;
			if (TakeInto(data, v))
			{
				return v;
			}
			break;
		}
		case BasicType::Vector3:
		{
			Vec3 v;
			if (TakeInto(data, v))
			{
				return v;
			}
			break;
		}
		case BasicType::String:
		case BasicType::UnicodeString:
		{
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
		}
		case BasicType::Blob:
		{
			// we need to copy the final data, because the entire data span will be cleared
			uint8_t size;
			if (!TakeInto(data, size))
			{
				break;
			}

			if (size == std::numeric_limits<uint8_t>::max())
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
					auto s = Take(data, blobSize);
					return std::vector<Byte>{ s.begin(), s.end() };
				}
			}
			else
			{
				if (data.size() >= size)
				{
					auto s = Take(data, size);
					return std::vector<Byte>{ s.begin(), s.end() };
				}
			}
			break;
		}
	}

	return PA_REPLAY_ERROR("Failed to parse ArgValue into PrimitiveType {}, only had {} bytes ({})", ToSting(type.Type), data.size(), Core::FormatBytes(data));
}

}

ReplayResult<ArgType> rp::ParseType(XMLElement* elem, const AliasType& aliases)
{
	static const std::unordered_map<std::string, BasicType> types
	{
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

		{ "MAILBOX", BasicType::Blob },  // TODO: these 2 can be parsed better
		{ "PYTHON", BasicType::Blob },  // TODO end
	};

	const std::string typeName = Core::String::ToUpper(Core::String::Trim(elem->GetText()));
	if (types.contains(typeName))
	{
		return PrimitiveType{ types.at(typeName) };
	}

	if (typeName == "ARRAY")
	{
		ArrayType arr{};
		if (XMLElement* ofElem = elem->FirstChildElement("of"))
		{
			PA_TRY(type, ParseType(ofElem, aliases));
			arr.SubType = std::make_shared<ArgType>(type);
		}

		if (XMLElement* sizeElem = elem->FirstChildElement("size"))
		{
			size_t size;
			if (Core::String::ParseNumber<size_t>(sizeElem->GetText(), size))
			{
				arr.Size = size;
			}
		}

		return arr;
	}

	if (typeName == "FIXED_DICT")
	{
		FixedDictType dict;

		if (XMLElement* allowNoneElem = elem->FirstChildElement("AllowNone"))
		{
			if (!Core::String::ParseBool(Core::String::Trim(allowNoneElem->GetText()), dict.AllowNone))
			{
				return PA_REPLAY_ERROR("Failed to parse bool for FixtedDictType");
			}
		}

		if (XMLElement* propElem = elem->FirstChildElement("Properties"))
		{
			for (XMLElement* prop = propElem->FirstChildElement(); prop != nullptr; prop = prop->NextSiblingElement())
			{
				if (XMLElement* typeElem = prop->FirstChildElement("Type"))
				{
					PA_TRY(type, ParseType(typeElem, aliases));
					dict.Properties.emplace_back(FixedDictProperty{ prop->Name(), std::make_shared<ArgType>(type) });
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
			PA_TRY(type, ParseType(ofElem, aliases));
			tuple.SubType = std::make_shared<ArgType>(type);
		}

		if (XMLElement* sizeElem = elem->FirstChildElement("size"))
		{
			size_t size;
			if (Core::String::ParseNumber<size_t>(sizeElem->GetText(), size))
			{
				tuple.Size = size;
			}
		}
		return tuple;
	}

	if (typeName == "USER_TYPE")
	{
		if (XMLElement* typeElem = elem->FirstChildElement("Type"))
		{
			// TODO: is this correct?
			bool isNullable = false;
			if (XMLElement* implElem = elem->FirstChildElement("implementedBy"))
			{
				isNullable = Core::String::StartsWith(implElem->GetText(), "NullableDef");
			}
			const char* text = typeElem->GetText();
			if (types.contains(text))
			{
				return UserType{ std::make_shared<ArgType>(PrimitiveType{ types.at(text) }), isNullable };
			}
			else if (aliases.contains(text))
			{
				return UserType{ std::make_shared<ArgType>(aliases.at(typeName)), isNullable };
			}
		}
		else
		{
			PrimitiveType type{ BasicType::Blob };
			return type;
		}
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
			return PrimitiveSize(arg.Type);
		}

		if constexpr (std::is_same_v<T, ArrayType>)
		{
			if (!arg.Size) return Infinity;
			size_t size = TypeSize(*arg.SubType);
			if (size == Infinity) return Infinity;
			return size * arg.Size.value();
		}

		if constexpr (std::is_same_v<T, FixedDictType>)
		{
			if (arg.AllowNone) return Infinity;
			size_t totalSize = 0;
			for (const FixedDictProperty& prop : arg.Properties)
			{
				const size_t size = TypeSize(*prop.Type);
				if (size == Infinity) return Infinity;
				totalSize += size;
			}
			return totalSize;
		}

		if constexpr (std::is_same_v<T, TupleType>)
		{
			size_t size = TypeSize(*arg.SubType);
			if (size == Infinity) return Infinity;
			return size * arg.Size;
		}

		if constexpr (std::is_same_v<T, UserType>)
		{
			return Infinity;
		}

		return Infinity;
	}, type);
}

#ifndef NDEBUG
std::string rp::PrintType(const ArgType& type)
{
	return std::visit([](auto&& t) -> std::string
	{
		using T = std::decay_t<decltype(t)>;
		if constexpr (std::is_same_v<T, PrimitiveType>)
		{
			switch (t.Type)
			{
				case BasicType::Uint8: return "uint8_t";
				case BasicType::Uint16: return "uint16_t";
				case BasicType::Uint32: return "uint32_t";
				case BasicType::Uint64: return "uint64_t";
				case BasicType::Int8: return "int8_t";
				case BasicType::Int16:return "int16_t";
				case BasicType::Int32: return "int32_t";
				case BasicType::Int64: return "int64_t";
				case BasicType::Float32: return "float32";
				case BasicType::Float64: return "float64";
				case BasicType::Vector2: return "Vector2";
				case BasicType::Vector3: return "Vector3";
				case BasicType::String: return "String";
				case BasicType::UnicodeString: return "UnicodeString";
				case BasicType::Blob: return "Blob";
			}
		}
		else if constexpr (std::is_same_v<T, ArrayType>)
		{
			if (t.Size)
				return fmt::format("Array<{}, {}>", t.Size.value(), PrintType(*t.SubType));
			return fmt::format("Array<-1, {}>", PrintType(*t.SubType));
		}
		else if constexpr (std::is_same_v<T, FixedDictType>)
		{
			std::string dict = fmt::format("FixedDict<{}, [", t.AllowNone);

			for (auto begin = t.Properties.begin(); begin != t.Properties.end();)
			{
				const FixedDictProperty p = *begin;
				dict += fmt::format("Property(name: {} type: {})", p.Name, PrintType(*p.Type));

				++begin;
				if (begin != t.Properties.end())
					dict += ", ";
			}
			dict += "]>";
			return dict;
		}
		else if constexpr (std::is_same_v<T, TupleType>)
		{
			return fmt::format("TupleType<{} - Size {}>", PrintType(*t.SubType), t.Size);
		}
		else if constexpr (std::is_same_v<T, UserType>)
		{
			return fmt::format("UserType<{}>", PrintType(*t.Type));
		}

		return "Unknown Type";
	}, type);
}
#endif

ReplayResult<ArgValue> rp::ParseValue(std::span<const Byte>& data, const ArgType& type)
{
	if (data.empty())
	{
		return PA_REPLAY_ERROR("ParseValue has empty data");
	}

	return std::visit([&data](auto&& t) -> ReplayResult<ArgValue>
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
			if (!t.Size)
			{
				if (!TakeInto(data, size))
				{
					return {};
				}
			}
			else
			{
				size = t.Size.value();
			}
			for (size_t i = 0; i < size; i++)
			{
				PA_TRY(value, ParseValue(data, *t.SubType));
				values.emplace_back(std::move(value));
			}
			return values;
		}
		else if constexpr (std::is_same_v<T, FixedDictType>)
		{
 			std::unordered_map<std::string, ArgValue> dict;
			if (t.AllowNone)
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

			for (const FixedDictProperty& property : t.Properties)
			{
				PA_TRY(value, ParseValue(data, *property.Type));
				dict.emplace(property.Name, std::move(value));
			}

			return dict;
		}
		else if constexpr (std::is_same_v<T, TupleType>)
		{
			std::vector<ArgValue> values;
			for (size_t i = 0; i < t.Size; i++)
			{
				PA_TRY(value, ParseValue(data, *t.SubType));
				values.emplace_back(std::move(value));
			}
			return values;
		}
		else if constexpr (std::is_same_v<T, UserType>)
		{
			if (const PrimitiveType* prim = std::get_if<PrimitiveType>(&*t.Type))
			{
				if (prim->Type == BasicType::Blob)
				{
					return ParseValue(data, *t.Type);
				}
			}
			if (data.size() == 0)
			{
				return PA_REPLAY_ERROR("UserType did not have size > 1");
			}
			Take(data, 1);
			if (t.IsNullable && data.size() == 0)  // TODO: is this correct?
			{
				return {};
			}
			return ParseValue(data, *t.Type);
		}
		return {};
	}, type);
}

ReplayResult<ArgValue> rp::GetDefaultValue(const ArgType& type)
{
	return std::visit([](auto&& t) -> ReplayResult<ArgValue>
	{
		using T = std::decay_t<decltype(t)>;
		if constexpr (std::is_same_v<T, PrimitiveType>)
		{
			switch (t.Type)
			{
				case BasicType::Uint8:
					return (uint8_t)0;
				case BasicType::Uint16:
					return (uint16_t)0;
				case BasicType::Uint32:
					return (uint32_t)0;
				case BasicType::Uint64:
					return (uint64_t)0;
				case BasicType::Int8:
					return (int8_t)0;
				case BasicType::Int16:
					return (int16_t)0;
				case BasicType::Int32:
					return (int32_t)0;
				case BasicType::Int64:
					return (int64_t)0;
				case BasicType::Float32:
					return 0.0f;
				case BasicType::Float64:
					return 0.0;
				case BasicType::Vector2:
					return Vec2{ 0, 0 };
				case BasicType::Vector3:
					return Vec3{ 0, 0, 0 };
				case BasicType::String:
					return std::string("");
				case BasicType::UnicodeString:
					return std::string("");
				case BasicType::Blob:
					return std::vector<Byte>{};
			}
		}
		else if constexpr (std::is_same_v<T, ArrayType>)
		{
			return std::vector<ArgValue>{};
		}
		else if constexpr (std::is_same_v<T, FixedDictType>)
		{
			return std::unordered_map<std::string, ArgValue>{};
		}
		else if constexpr (std::is_same_v<T, TupleType>)
		{
			return PA_REPLAY_ERROR("TupleType encountered in GetDefault");
		}
		else if constexpr (std::is_same_v<T, UserType>)
		{
			return GetDefaultValue(*t.Type);
		}

		return ArgValue{};
	}, type);
}
