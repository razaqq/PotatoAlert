// Copyright 2023 <github.com/razaqq>
#pragma once

#include "ReplayParser/Types.hpp"
#include "ReplayParser/Result.hpp"
#include "ReplayParser/Variant.hpp"

#include <filesystem>
#include <unordered_map>
#include <source_location>
#include <string>
#include <variant>
#include <vector>


namespace PotatoAlert::ReplayParser {

using ReplayParser::ArgValue;
using ReplayParser::VariantGet;

using KeyType = std::variant<std::string_view, size_t, int>;

using ArrayValue = std::vector<ArgValue>;
using DictValue = std::unordered_map<std::string, ArgValue>;

template<typename Value>
inline ReplayResult<Value> GetArgValue(const ArgValue& value, std::initializer_list<KeyType> keys = {}, const std::source_location loc = std::source_location::current())
{
	const ArgValue* current = &value;
	for (KeyType key : keys)
	{
		PA_TRYV(std::visit([&current, loc](auto&& k) -> ReplayResult<void>
		{
			using T = std::decay_t<decltype(k)>;
			if constexpr (std::is_same_v<T, std::string_view>)
			{
				PA_TRYV(VariantGet<DictValue>(*current, [&current, k, loc](const DictValue& v) -> ReplayResult<void>
				{
					if (v.contains(k.data()))
					{
						current = &v.at(k.data());
						return {};
					}
					return PA_REPLAY_ERROR("{}:{} - DictValue was missing key '{}'", std::filesystem::path(loc.file_name()).filename(), loc.line(), k);
				}));
			}
			else if constexpr (std::is_same_v<T, size_t> || std::is_same_v<T, int>)
			{
				PA_TRYV(VariantGet<ArrayValue>(*current, [&current, k, loc](const ArrayValue& v) -> ReplayResult<void>
				{
					if ((size_t)k < v.size())
					{
						current = &v[k];
						return {};
					}
					return PA_REPLAY_ERROR("{}:{} - ArrayValue was size {}, but tried to access index {}", std::filesystem::path(loc.file_name()).filename(), loc.line(), v.size(), k);
				}));
			}

			return {};
		}, key));
	}

	ReplayResult<void> result;
	Value outValue;
	if constexpr (std::is_enum_v<Value>)
	{
		result = VariantGet<std::underlying_type_t<Value>>(*current, [&outValue](auto v) -> ReplayResult<void>
		{
			outValue = static_cast<Value>(v);
			return {};
		});
		if (!result)
			return PA_REPLAY_ERROR("{}:{} - {}", std::filesystem::path(loc.file_name()).filename(), loc.line(), result.error());
	}
	else
	{
		result = VariantGet<Value>(*current, [&outValue](const auto& v) -> ReplayResult<void>
		{
			outValue = v;
			return {};
		});
		if (!result)
			return PA_REPLAY_ERROR("{}:{} - {}", std::filesystem::path(loc.file_name()).filename(), loc.line(), result.error());
	}
	return outValue;
}

}  // namespace PotatoAlert::ReplayParser
