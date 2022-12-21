// Copyright 2021 <github.com/razaqq>
#pragma once

#include "ReplayParser/GameFiles.hpp"
#include "ReplayParser/Packets.hpp"

#include <span>
#include <unordered_map>
#include <variant>
#include <vector>


namespace PotatoAlert::ReplayParser {

struct Entity
{
	uint16_t Type;
	std::vector<ArgValue> Properties;
};

struct PacketParser
{
	std::vector<EntitySpec> Specs;
	std::unordered_map<uint32_t, Entity> Entities;
};

PacketType ParsePacket(std::span<const Byte>& data, PacketParser& parser);

std::variant<EntityMethodPacket, InvalidPacket> ParseEntityMethodPacket(std::span<const Byte>& data, PacketParser& parser, float clock);
std::variant<EntityCreatePacket, InvalidPacket> ParseEntityCreatePacket(std::span<const Byte>& data, PacketParser& parser, float clock);
std::variant<EntityPropertyPacket, InvalidPacket> ParseEntityPropertyPacket(std::span<const Byte>& data, const PacketParser& parser, float clock);
std::variant<BasePlayerCreatePacket, InvalidPacket> ParseBasePlayerCreatePacket(std::span<const Byte>& data, PacketParser& parser, float clock);
std::variant<CellPlayerCreatePacket, InvalidPacket> ParseCellPlayerCreatePacket(std::span<const Byte>& data, const PacketParser& parser, float clock);
std::variant<EntityControlPacket, InvalidPacket> ParseEntityControlPacket(std::span<const Byte>& data, float clock);
std::variant<EntityEnterPacket, InvalidPacket> ParseEntityEnterPacket(std::span<const Byte>& data, float clock);
std::variant<EntityLeavePacket, InvalidPacket> ParseEntityLeavePacket(std::span<const Byte>& data, float clock);
std::variant<NestedPropertyUpdatePacket, InvalidPacket> ParseNestedPropertyUpdatePacket(std::span<const Byte>& data, PacketParser& parser, float clock);
std::variant<PlayerOrientationPacket, InvalidPacket> ParsePlayerOrientationPacket(std::span<const Byte>& data, float clock);
std::variant<PlayerPositionPacket, InvalidPacket> ParsePlayerPositionPacketPacket(std::span<const Byte>& data, float clock);
std::variant<CameraPacket, InvalidPacket> ParseCameraPacket(std::span<const Byte>& data, float clock);
std::variant<MapPacket, InvalidPacket> ParseMapPacket(std::span<const Byte>& data, float clock);
std::variant<VersionPacket, InvalidPacket> ParseVersionPacket(std::span<const Byte>& data, float clock);
std::variant<PlayerEntityPacket, InvalidPacket> ParsePlayerEntityPacket(std::span<const Byte>& data, float clock);
std::variant<CameraModePacket, InvalidPacket> ParseCameraModePacket(std::span<const Byte>& data, float clock);
std::variant<CruiseStatePacket, InvalidPacket> ParseCruiseStatePacket(std::span<const Byte>& data, float clock);
std::variant<CameraFreeLookPacket, InvalidPacket> ParseCameraFreeLookPacket(std::span<const Byte>& data, float clock);

}  // namespace PotatoAlert::ReplayParser
