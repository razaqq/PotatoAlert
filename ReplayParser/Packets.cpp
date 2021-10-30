// Copyright 2021 <github.com/razaqq>

#include "Packets.hpp"

#include "ByteUtil.hpp"

#include <optional>
#include <span>


using namespace PotatoAlert::ReplayParser;

std::optional<RawPacket> RawPacket::FromBytes(std::span<std::byte>& data)
{
	RawPacket rawPacket;
	uint32_t size;
	if (!TakeInto(data, size))
		return {};
	if (!TakeInto(data, rawPacket.type))
		return {};
	if (!TakeInto(data, rawPacket.clock))
		return {};

	rawPacket.raw.reserve(size);
	if (data.size() >= size)
	{
		std::memcpy(rawPacket.raw.data(), Take(data, size).data(), size);
		return rawPacket;
	}

	return {};
}
