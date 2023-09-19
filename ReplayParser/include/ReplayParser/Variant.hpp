// Copyright 2022 <github.com/razaqq>
#pragma once

#include "ReplayParser/Packets.hpp"
#include "ReplayParser/Result.hpp"
#include "ReplayParser/Types.hpp"

#include <expected>
#include <string>


namespace PotatoAlert::ReplayParser {

template<typename T>
inline constexpr ReplayResult<void> VariantGet(const ArgValue& value, auto&& then)
{
	if (const T* v = std::get_if<T>(&value))
	{
		return then(*v);
	}
	else
	{
		return PA_REPLAY_ERROR("Failed to get type '{}' from ArgValue", typeid(T).name());
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

	if (const T* v = std::get_if<T>(&packet.Values[index]))
	{
		return then(*v);
	}
	else
	{
		return PA_REPLAY_ERROR("ValueType (index {}) for EntityMethodPacket '{}' did not match '{}' and instead was '{}'",
				  index, packet.MethodName, typeid(T).name(), VariantType(packet.Values[index]).name());
	}
}

}  // namespace PotatoAlert::ReplayParser
