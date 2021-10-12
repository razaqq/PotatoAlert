// Copyright 2021 <github.com/razaqq>

#include "Packets.hpp"

#include "ByteUtil.hpp"

#include <optional>


using PotatoAlert::ReplayParser::RawPacket;
using PotatoAlert::ReplayParser::EntityMethodPacket;
using PotatoAlert::ReplayParser::InvalidPacket;
using PotatoAlert::ReplayParser::TakeInto;

std::optional<RawPacket> RawPacket::FromBytes(std::span<std::byte>& data)
{
	RawPacket rawPacket;
	if (!TakeInto(data, rawPacket.size))
		return {};
	if (!TakeInto(data, rawPacket.type))
		return {};
	if (!TakeInto(data, rawPacket.clock))
		return {};

	rawPacket.raw.resize(rawPacket.size);
	if (data.size() >= rawPacket.size)
	{
		std::memcpy(rawPacket.raw.data(), Take(data, rawPacket.size).data(), rawPacket.size);
		return rawPacket;
	}

	return {};
}


std::variant<EntityMethodPacket, InvalidPacket> EntityMethodPacket::Parse(float clock, std::span<std::byte>& data)
{
	EntityMethodPacket packet;
	packet.type = PacketType::EntityMethod;
	packet.clock = clock;

	if (!TakeInto(data, packet.entityId))
		return InvalidPacket{};  // TODO: raw data
	if (!TakeInto(data, packet.methodId))
		return InvalidPacket{};
	if (!TakeInto(data, packet.size))
		return InvalidPacket{};

	packet.data.resize(packet.size);
	if (data.size() >= packet.size)
	{
		std::memcpy(packet.data.data(), Take(data, packet.size).data(), packet.size);
		return packet;
	}

	return InvalidPacket{};
}
