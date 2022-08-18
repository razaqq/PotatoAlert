// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Core/Bytes.hpp"
#include "Core/Math.hpp"

#include <memory>
#include <tinyxml2.h>

#include <any>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>


using PotatoAlert::Core::Vec2;
using PotatoAlert::Core::Vec3;

namespace PotatoAlert::ReplayParser {

using namespace tinyxml2;

constexpr size_t Infinity = 0xFFFF;

enum class BasicType
{
	// Primitive
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

inline size_t PrimitiveSize(BasicType type)
{
	switch (type)
	{
		case BasicType::Uint8: return sizeof(uint8_t);
		case BasicType::Uint16: return sizeof(uint16_t);
		case BasicType::Uint32: return sizeof(uint32_t);
		case BasicType::Uint64: return sizeof(uint64_t);
		case BasicType::Int8: return sizeof(int8_t);
		case BasicType::Int16: return sizeof(int16_t);
		case BasicType::Int32: return sizeof(int32_t);
		case BasicType::Int64: return sizeof(int64_t);
		case BasicType::Float32: return sizeof(float);
		case BasicType::Float64: return sizeof(double);
		case BasicType::Vector2: return 2 * PrimitiveSize(BasicType::Float32);
		case BasicType::Vector3: return 3 * PrimitiveSize(BasicType::Float32);
		case BasicType::String:
		case BasicType::UnicodeString:
		case BasicType::Blob: return Infinity;
	}
	return Infinity;
}

struct PrimitiveType;
struct ArrayType;
struct FixedDictType;
struct TupleType;
struct UnknownType;
typedef std::variant<PrimitiveType, ArrayType, FixedDictType, TupleType, UnknownType> ArgType;
typedef std::any ArgValue;

struct PrimitiveType
{
	BasicType Type;
};

struct ArrayType
{
	std::shared_ptr<ArgType> SubType;
	std::optional<size_t> Size;
};

struct FixedDictProperty
{
	std::string Name;
	std::shared_ptr<ArgType> Type;
};

struct FixedDictType
{
	bool AllowNone = false;
	std::vector<FixedDictProperty> Properties = {};
};

struct TupleType
{
	std::shared_ptr<ArgType> SubType;
	size_t Size;
};

struct UnknownType {};

typedef std::unordered_map<std::string, ArgType> AliasType;

ArgType ParseType(XMLElement* elem, const AliasType& aliases);
size_t TypeSize(const ArgType& type);
ArgValue ParseValue(std::span<Core::Byte>& data, const ArgType& type);

#ifndef NDEBUG
std::string PrintType(const ArgType& type);
#endif

}  // namespace PotatoAlert::ReplayParser
