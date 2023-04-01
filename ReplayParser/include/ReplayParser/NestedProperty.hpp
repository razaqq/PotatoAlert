// Copyright 2022 <github.com/razaqq>
#pragma once

#include "ReplayParser/BitReader.hpp"
#include "ReplayParser/Types.hpp"
#include "ReplayParser/Result.hpp"

#include <string>
#include <variant>
#include <vector>


namespace PotatoAlert::ReplayParser {

using PropertyNestLevelVariant = std::variant<size_t, std::string>;
struct PropertyNestLevel : PropertyNestLevelVariant
{
	using PropertyNestLevelVariant::PropertyNestLevelVariant;
};

struct UpdateActionSetKey
{
	std::string Key;
	ArgValue Value;
};

struct UpdateActionSetRange
{
	size_t Start;
	size_t Stop;
	std::vector<ArgValue> Values;
};

struct UpdateActionRemoveRange
{
	size_t Start;
	size_t Stop;
};

struct UpdateActionSetElement
{
	size_t Index;
	ArgValue Value;
};

using UpdateActionVariant = std::variant<UpdateActionSetKey, UpdateActionSetRange, UpdateActionRemoveRange, UpdateActionSetElement>;
struct UpdateAction : UpdateActionVariant
{
	using UpdateActionVariant::UpdateActionVariant;
};

struct PropertyNesting
{
	std::vector<PropertyNestLevel> Levels;
	UpdateAction Action;
};

ReplayResult<PropertyNesting> GetNestedPropertyPath(bool isSlice, const ArgType& argType, ArgValue* argValue, BitReader& bitReader);

}  // namespace PotatoAlert::ReplayParser
