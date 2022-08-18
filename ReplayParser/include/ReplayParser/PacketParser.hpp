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
	uint16_t Type;
	std::vector<ArgValue> Properties;
};

struct PacketParser
{
	std::vector<EntitySpec> Specs;
	std::unordered_map<uint32_t, Entity> Entities;
};

PacketType ParsePacket(std::span<Byte>& data, PacketParser& parser);

std::variant<EntityMethodPacket, InvalidPacket> ParseEntityMethodPacket(std::span<Byte>& data, PacketParser& parser, float clock);
std::variant<EntityCreatePacket, InvalidPacket> ParseEntityCreatePacket(std::span<Byte>& data, PacketParser& parser, float clock);
std::variant<EntityPropertyPacket, InvalidPacket> ParseEntityPropertyPacket(std::span<Byte>& data, const PacketParser& parser, float clock);
std::variant<BasePlayerCreatePacket, InvalidPacket> ParseBasePlayerCreatePacket(std::span<Byte>& data, PacketParser& parser, float clock);
std::variant<CellPlayerCreatePacket, InvalidPacket> ParseCellPlayerCreatePacket(std::span<Byte>& data, const PacketParser& parser, float clock);
std::variant<EntityControlPacket, InvalidPacket> ParseEntityControlPacket(std::span<Byte>& data, float clock);
std::variant<EntityEnterPacket, InvalidPacket> ParseEntityEnterPacket(std::span<Byte>& data, float clock);
std::variant<EntityLeavePacket, InvalidPacket> ParseEntityLeavePacket(std::span<Byte>& data, float clock);
std::variant<NestedPropertyUpdatePacket, InvalidPacket> ParseNestedPropertyUpdatePacket(std::span<Byte>& data, PacketParser& parser, float clock);
std::variant<PlayerOrientationPacket, InvalidPacket> ParsePlayerOrientationPacket(std::span<Byte>& data, float clock);
std::variant<PlayerPositionPacket, InvalidPacket> ParsePlayerPositionPacketPacket(std::span<Byte>& data, float clock);
std::variant<CameraPacket, InvalidPacket> ParseCameraPacket(std::span<Byte>& data, float clock);
std::variant<MapPacket, InvalidPacket> ParseMapPacket(std::span<Byte>& data, float clock);
std::variant<VersionPacket, InvalidPacket> ParseVersionPacket(std::span<Byte>& data, float clock);
std::variant<PlayerEntityPacket, InvalidPacket> ParsePlayerEntityPacket(std::span<Byte>& data, float clock);
std::variant<CameraModePacket, InvalidPacket> ParseCameraModePacket(std::span<Byte>& data, float clock);
std::variant<CruiseStatePacket, InvalidPacket> ParseCruiseStatePacket(std::span<Byte>& data, float clock);
std::variant<CameraFreeLookPacket, InvalidPacket> ParseCameraFreeLookPacket(std::span<Byte>& data, float clock);

}  // namespace PotatoAlert::ReplayParser
