// Copyright 2021 <github.com/razaqq>
#pragma once

#include "GameFiles.hpp"
#include "Packets.hpp"

#include <span>
#include <unordered_map>
#include <variant>
#include <vector>


namespace PotatoAlert::ReplayParser {

struct Entity
{
	uint16_t type;
	std::vector<ArgValue> properties;
};

struct PacketParser
{
	std::vector<EntitySpec> specs;
	std::unordered_map<uint32_t, Entity> entities;
};

PacketType ParsePacket(std::span<std::byte>& data, PacketParser& parser);

std::variant<EntityMethodPacket, InvalidPacket> ParseEntityMethodPacket(std::span<std::byte>& data, PacketParser& parser, float clock);
std::variant<EntityCreatePacket, InvalidPacket> ParseEntityCreatePacket(std::span<std::byte>& data, PacketParser& parser, float clock);
std::variant<EntityPropertyPacket, InvalidPacket> ParseEntityPropertyPacket(std::span<std::byte>& data, const PacketParser& parser, float clock);
std::variant<BasePlayerCreatePacket, InvalidPacket> ParseBasePlayerCreatePacket(std::span<std::byte>& data, PacketParser& parser, float clock);
std::variant<CellPlayerCreatePacket, InvalidPacket> ParseCellPlayerCreatePacket(std::span<std::byte>& data, const PacketParser& parser, float clock);
std::variant<EntityControlPacket, InvalidPacket> ParseEntityControlPacket(std::span<std::byte>& data, float clock);
std::variant<EntityEnterPacket, InvalidPacket> ParseEntityEnterPacket(std::span<std::byte>& data, float clock);
std::variant<EntityLeavePacket, InvalidPacket> ParseEntityLeavePacket(std::span<std::byte>& data, float clock);
std::variant<NestedPropertyUpdatePacket, InvalidPacket> ParseNestedPropertyUpdatePacket(std::span<std::byte>& data, PacketParser& parser, float clock);
std::variant<PlayerOrientationPacket, InvalidPacket> ParsePlayerOrientationPacket(std::span<std::byte>& data, float clock);
std::variant<PlayerPositionPacket, InvalidPacket> ParsePlayerPositionPacketPacket(std::span<std::byte>& data, float clock);
std::variant<CameraPacket, InvalidPacket> ParseCameraPacket(std::span<std::byte>& data, float clock);
std::variant<MapPacket, InvalidPacket> ParseMapPacket(std::span<std::byte>& data, float clock);
std::variant<VersionPacket, InvalidPacket> ParseVersionPacket(std::span<std::byte>& data, float clock);
std::variant<PlayerEntityPacket, InvalidPacket> ParsePlayerEntityPacket(std::span<std::byte>& data, float clock);

}  // namespace PotatoAlert::ReplayParser
