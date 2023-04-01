#pragma once

#include "Core/Preprocessor.hpp"

#include "ReplayParser/Packets.hpp"

#include <functional>
#include <variant>
#include <vector>

namespace PotatoAlert::ReplayParser {

template<class T, class U>
struct IsInVariant;

template<class T, class... Ts>
struct IsInVariant<T, std::variant<Ts...>>
	: std::bool_constant<(std::is_same_v<T, Ts> || ...)>
{
};

class PacketCallbacks
{
#define PA_DEFINE_PACKET_INVOKE(Packet)                        \
	void Invoke(const Packet& packet) const                    \
	{                                                          \
		static_assert(IsInVariant<Packet, PacketType>::value); \
		for (const auto& callback : m_##Packet##Callbacks)     \
		{                                                      \
			callback(packet);                                  \
		}                                                      \
	}

#define PA_DEFINE_PACKET_ADD(Packet)                             \
	void Add(std::function<void(const Packet& packet)> callback) \
	{                                                            \
		static_assert(IsInVariant<Packet, PacketType>::value);   \
		m_##Packet##Callbacks.emplace_back(callback);            \
	}

#define X(Packet)                                                          \
private:                                                                   \
	std::vector<std::function<void(const Packet&)>> m_##Packet##Callbacks; \
                                                                           \
public:                                                                    \
	PA_DEFINE_PACKET_INVOKE(Packet)                                        \
	PA_DEFINE_PACKET_ADD(Packet)

	PA_RP_PACKETS(X)

#undef PA_DEFINE_PACKET_INVOKE
#undef PA_DEFINE_PACKET_ADD
#undef X
};

}  // namespace PotatoAlert::ReplayParser
