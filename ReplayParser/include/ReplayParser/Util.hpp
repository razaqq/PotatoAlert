// Copyright 2022 <github.com/razaqq>
#pragma once

#include <Core/Result.hpp>

#include "ReplayParser/Types.hpp"

#include <expected>
#include <format>


namespace PotatoAlert::ReplayParser {

using ReplayError = std::string;
template<typename T>
using ReplayResult = Core::Result<T, ReplayError>;
#define PA_REPLAY_ERROR(...) (::std::unexpected(::PotatoAlert::ReplayParser::ReplayError(std::format(__VA_ARGS__))))

template<typename T>
inline constexpr void VariantGet(const ArgValue& value, auto&& then)
{
	if (const T* Name = std::get_if<T>(&value))
	{
		then(*Name);
	}
	else
	{
		LOG_ERROR("Failed to get type '{}' from ArgValue", typeid(T).name());
	}
}

template<typename V>
const std::type_info& VariantType(const V& v)
{
	return std::visit([](auto&& x) -> decltype(auto)
	{
		return typeid(x);
	}, v);
}

template<typename T>
static constexpr ReplayResult<void> VariantGet(const EntityMethodPacket& packet, size_t index, auto&& then)
{
	if (packet.Values.size() <= index)
	{
		return PA_REPLAY_ERROR("Index out of range for EntityMethodPacket '{}' (index {}, size {})",
				  packet.MethodName, index, packet.Values.size());
	}

	if (const T* Name = std::get_if<T>(&packet.Values[index]))
	{
		return then(*Name);
	}
	else
	{
		return PA_REPLAY_ERROR("ValueType (index {}) for EntityMethodPacket '{}' did not match '{}' and instead was '{}'",
				  index, packet.MethodName, typeid(T).name(), VariantType(packet.Values[index]).name());
	}
}

}  // namespace PotatoAlert::ReplayParser
