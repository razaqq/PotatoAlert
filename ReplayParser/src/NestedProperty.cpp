// Copyright 2022 <github.com/razaqq>

#include "Core/Bytes.hpp"
#include "Core/Log.hpp"

#include "ReplayParser/BitReader.hpp"
#include "ReplayParser/NestedProperty.hpp"
#include "ReplayParser/Result.hpp"
#include "ReplayParser/Types.hpp"
#include "ReplayParser/Variant.hpp"

#include <type_traits>
#include <span>


namespace rp = PotatoAlert::ReplayParser;
using PotatoAlert::ReplayParser::ArrayType;
using PotatoAlert::ReplayParser::ArgType;
using PotatoAlert::ReplayParser::ArgValue;
using PotatoAlert::ReplayParser::BitReader;
using PotatoAlert::ReplayParser::FixedDictType;
using PotatoAlert::ReplayParser::FixedDictProperty;
using PotatoAlert::ReplayParser::UpdateActionSetKey;
using PotatoAlert::ReplayParser::UpdateActionRemoveRange;
using PotatoAlert::ReplayParser::UpdateActionSetRange;
using PotatoAlert::ReplayParser::UpdateActionSetElement;
using PotatoAlert::ReplayParser::PropertyNesting;
using PotatoAlert::ReplayParser::PropertyNestLevel;
using PotatoAlert::ReplayParser::ReplayResult;

namespace {

ReplayResult<PropertyNesting> GetNestedUpdateCommand(bool isSlice, const ArgType& argType, ArgValue* argValue, BitReader& bitReader)
{
	return std::visit([isSlice, &bitReader, &argValue](auto&& arg) -> ReplayResult<PropertyNesting>
	{
		using T = std::decay_t<decltype(arg)>;

		if constexpr (std::is_same_v<T, FixedDictType>)
		{
			int entryIndex = bitReader.Get(BitReader::BitsRequired(arg.Properties.size()));
			while (bitReader.Remaining() % 8 != 0)
			{
				bitReader.Get(1);
			}

			std::span<const Byte> remaining = bitReader.GetAll();

			const FixedDictProperty prop = arg.Properties[entryIndex];
			const ArgValue propValue = ParseValue(remaining, *prop.Type);

			// we can safely use std::get here
			std::unordered_map<std::string, ArgValue>& value = std::get<std::unordered_map<std::string, ArgValue>>(*argValue);
			value.insert_or_assign(prop.Name, propValue);

			return PropertyNesting{ {}, UpdateActionSetKey{ prop.Name, propValue } };
		}
		else if constexpr (std::is_same_v<T, ArrayType>)
		{
			// we can safely use std::get here
			std::vector<ArgValue>& value = std::get<std::vector<ArgValue>>(*argValue);

			if (isSlice)
			{
				const int idxBits = BitReader::BitsRequired(value.size() + 1);
				const size_t idx1 = bitReader.Get(idxBits);
				const size_t idx2 = bitReader.Get(idxBits);
				if (idx1 > idx2)
					return PA_REPLAY_ERROR("FixedDict ArrayType has idx1 > idx2: {} > {}", idx1, idx2);

				while (bitReader.Remaining() % 8 != 0)
				{
					bitReader.Get(1);
				}

				std::span<const Byte> remaining = bitReader.GetAll();

				auto SliceInsert = []<typename T>(size_t idx1, size_t idx2, std::vector<T>& target, const std::vector<T>& source)
				{
					if (idx1 != idx2)
					{
						target.erase(target.begin() + idx1, target.begin() + idx2);
					}

					for (size_t i = 0; i < source.size(); i++)
					{
						target.insert(target.begin() + std::min(idx1 + i, target.size()), source[i]);
					}
				};

				if (remaining.empty())
				{
					const std::vector<ArgValue> a{};
					SliceInsert(idx1, idx2, value, a);
					return PropertyNesting{ {}, UpdateActionRemoveRange{ idx1, idx2 } };
				}

				std::vector<ArgValue> newValues;
				while (!remaining.empty())
				{
					newValues.emplace_back(ParseValue(remaining, *arg.SubType));
				}

				SliceInsert(idx1, idx2, value, newValues);
				return PropertyNesting{ {}, UpdateActionSetRange{ idx1, idx2, newValues } };
			}
			else
			{
				const int idxBits = BitReader::BitsRequired(value.size());
				const size_t index = bitReader.Get(idxBits);

				while (bitReader.Remaining() % 8 != 0)
				{
					bitReader.Get(1);
				}

				std::span<const Byte> remaining = bitReader.GetAll();
				if (remaining.empty())
					return PA_REPLAY_ERROR("FixedDict ArrayType has no data remaining");

				std::vector<ArgValue> newValues;
				while (!remaining.empty())
				{
					newValues.emplace_back(ParseValue(remaining, *arg.SubType));
				}
				newValues.erase(newValues.begin());

				value[index] = newValues;

				return PropertyNesting{ {}, UpdateActionSetElement{ index, value[index] } };
			}
		}
		else
		{
			return PA_REPLAY_ERROR("Nested Property is neither FixedDictType nor ArrayType, but instead '{}'", typeid(T).name());
		}
	}, argType);
}

}  // namespace


ReplayResult<PropertyNesting> rp::GetNestedPropertyPath(bool isSlice, const ArgType& argType, ArgValue* argValue, BitReader& bitReader)
{
	if (bitReader.Get(1) == 0)
	{
		return GetNestedUpdateCommand(isSlice, argType, argValue, bitReader);
	}

	return std::visit([isSlice, &bitReader, &argValue](auto&& arg) -> ReplayResult<PropertyNesting>
	{
		using T = std::decay_t<decltype(arg)>;

		if constexpr (std::is_same_v<T, FixedDictType>)
		{
			const int propIndex = bitReader.Get(BitReader::BitsRequired(arg.Properties.size()));
			const FixedDictProperty& prop = arg.Properties[propIndex];

			const ArgValue* newArgValue;

			ReplayResult<void> setRes = VariantGet<std::unordered_map<std::string, ArgValue>>(*argValue, [&prop, &newArgValue, &arg](const auto& value) -> ReplayResult<void>
			{
				if (value.contains(prop.Name))
				{
					newArgValue = &value.at(prop.Name);
					return {};
				}
				return PA_REPLAY_ERROR("Nested Property Path does not contain property named ''", prop.Name);
			});
			if (!setRes)
				return ReplayResult<PropertyNesting>{ Core::ResultError, ReplayError(setRes.error()) };
			argValue = const_cast<ArgValue*>(newArgValue);

			const ArgType newType = *arg.Properties[propIndex].Type;

			PA_TRY(nesting, GetNestedPropertyPath(isSlice, *arg.Properties[propIndex].Type, argValue, bitReader));
			nesting.Levels.insert(nesting.Levels.begin(), PropertyNestLevel{ prop.Name });
			return nesting;
		}
		else if constexpr (std::is_same_v<T, ArrayType>)
		{
			// this always has to be std::vector<ArgValue>
			std::vector<ArgValue>& arr = std::get<std::vector<ArgValue>>(*argValue);
			const size_t propIndex = bitReader.Get(BitReader::BitsRequired(static_cast<int>(arr.size())));

			if (propIndex == arr.size())
			{
				arr.push_back(GetDefaultValue(*arg.SubType));
			}
			else if (propIndex > arr.size())
			{
				LOG_ERROR("Array property accessed out of bounds (index {} - size {})", propIndex, arr.size());
				return PropertyNesting{};  // TODO: handle this properly
			}

			PA_TRY(nesting, GetNestedPropertyPath(isSlice, *arg.SubType, &arr[propIndex], bitReader));
			nesting.Levels.insert(nesting.Levels.begin(), PropertyNestLevel{ propIndex });
			return nesting;
		}
		else
		{
			return PA_REPLAY_ERROR("Nested Property is neither FixedDictType nor ArrayType, but instead '{}'", typeid(T).name());
		}
	}, argType);
}
