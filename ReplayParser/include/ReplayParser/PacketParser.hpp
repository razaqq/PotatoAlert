// Copyright 2021 <github.com/razaqq>
#pragma once

#include "ReplayParser/Entity.hpp"
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
	EntityType Type;
	std::reference_wrapper<const EntitySpec> Spec;
	std::unordered_map<std::string, ArgValue> BasePropertiesValues;
	std::unordered_map<std::string, ArgValue> ClientPropertiesValues;
	std::unordered_map<std::string, ArgValue> ClientPropertiesInternalValues;
};

struct PacketParser
{
	std::vector<EntitySpec> Specs;
	std::unordered_map<TypeEntityId, Entity> Entities;
	PacketCallbacks Callbacks;
};

ReplayResult<PacketType> ParsePacket(std::span<const Byte>& data, PacketParser& parser, Core::Version version);

}  // namespace PotatoAlert::ReplayParser
