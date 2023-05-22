// Copyright 2021 <github.com/razaqq>
#pragma once

#include "ReplayParser/GameFiles.hpp"
#include "ReplayParser/Packets.hpp"
#include "ReplayParser/PacketCallback.hpp"
#include "ReplayParser/Result.hpp"

#include <span>
#include <unordered_map>
#include <variant>
#include <vector>


namespace PotatoAlert::ReplayParser {

struct Entity
{
	uint16_t Type;
	std::reference_wrapper<const EntitySpec> Spec;
	std::unordered_map<std::string, ArgValue> BasePropertiesValues;
	std::unordered_map<std::string, ArgValue> ClientPropertiesValues;
	std::unordered_map<std::string, ArgValue> ClientPropertiesInternalValues;
};

struct PacketParser
{
	std::vector<EntitySpec> Specs;
	std::unordered_map<uint32_t, Entity> Entities;
	PacketCallbacks Callbacks;
};

ReplayResult<PacketType> ParsePacket(std::span<const Byte>& data, PacketParser& parser);

ReplayResult<EntityMethodPacket> ParseEntityMethodPacket(std::span<const Byte>& data, PacketParser& parser, float clock);
ReplayResult<EntityCreatePacket> ParseEntityCreatePacket(std::span<const Byte>& data, PacketParser& parser, float clock);
ReplayResult<EntityPropertyPacket> ParseEntityPropertyPacket(std::span<const Byte>& data, PacketParser& parser, float clock);
ReplayResult<BasePlayerCreatePacket> ParseBasePlayerCreatePacket(std::span<const Byte>& data, PacketParser& parser, float clock);
ReplayResult<CellPlayerCreatePacket> ParseCellPlayerCreatePacket(std::span<const Byte>& data, PacketParser& parser, float clock);
ReplayResult<EntityControlPacket> ParseEntityControlPacket(std::span<const Byte>& data, const PacketParser& parser, float clock);
ReplayResult<EntityEnterPacket> ParseEntityEnterPacket(std::span<const Byte>& data, const PacketParser& parser, float clock);
ReplayResult<EntityLeavePacket> ParseEntityLeavePacket(std::span<const Byte>& data, const PacketParser& parser, float clock);
ReplayResult<NestedPropertyUpdatePacket> ParseNestedPropertyUpdatePacket(std::span<const Byte>& data, PacketParser& parser, float clock);
ReplayResult<PlayerOrientationPacket> ParsePlayerOrientationPacket(std::span<const Byte>& data, const PacketParser& parser, float clock);
ReplayResult<PlayerPositionPacket> ParsePlayerPositionPacketPacket(std::span<const Byte>& data, const PacketParser& parser, float clock);
ReplayResult<CameraPacket> ParseCameraPacket(std::span<const Byte>& data, const PacketParser& parser, float clock);
ReplayResult<MapPacket> ParseMapPacket(std::span<const Byte>& data, const PacketParser& parser, float clock);
ReplayResult<VersionPacket> ParseVersionPacket(std::span<const Byte>& data, const PacketParser& parser, float clock);
ReplayResult<PlayerEntityPacket> ParsePlayerEntityPacket(std::span<const Byte>& data, const PacketParser& parser, float clock);
ReplayResult<CameraModePacket> ParseCameraModePacket(std::span<const Byte>& data, const PacketParser& parser, float clock);
ReplayResult<CruiseStatePacket> ParseCruiseStatePacket(std::span<const Byte>& data, const PacketParser& parser, float clock);
ReplayResult<CameraFreeLookPacket> ParseCameraFreeLookPacket(std::span<const Byte>& data, const PacketParser& parser, float clock);

}  // namespace PotatoAlert::ReplayParser
