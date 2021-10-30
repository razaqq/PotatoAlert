// Copyright 2021 <github.com/razaqq>
#pragma once

#include "ByteUtil.hpp"
#include "Packets.hpp"


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

inline std::variant<EntityMethodPacket, InvalidPacket> ParseEntityMethodPacket(std::span<std::byte>& data, PacketParser& parser, float clock)
{
	EntityMethodPacket packet;
	packet.type = PacketBaseType::EntityMethod;
	packet.clock = clock;

	auto err = [data]() -> InvalidPacket
	{
		LOG_ERROR("Failed to parse EntityMethodPacket: {}", FormatBytes(data));
		return InvalidPacket{};
	};

	if (!TakeInto(data, packet.entityId))
		return err();
	if (!TakeInto(data, packet.methodId))
		return err();

	uint32_t size;
	if (!TakeInto(data, size))
		return err();

	if (data.size() != size)
	{
		LOG_ERROR("Invalid payload size on EntityMethodPacket: {} != {}", data.size(), size);
		return InvalidPacket{};
	}

	if (!parser.entities.contains(packet.entityId))
	{
		LOG_ERROR("Entity {} does not exist for EntityMethodPacket", packet.entityId);
		return InvalidPacket{};
	}
	const uint16_t entityType = parser.entities.at(packet.entityId).type;

	const int specId = entityType - 1;
	if (specId < 0 || specId >= parser.specs.size())
	{
		LOG_ERROR("Missing EntitySpec {} for EntityMethodPacket", specId);
		return InvalidPacket{};
	}
	EntitySpec& spec = parser.specs[specId];

	if (packet.methodId >= spec.clientMethods.size())
	{
		LOG_ERROR("Invalid methodId {} for EntityMethodPacket", packet.methodId);
		return InvalidPacket{};
	}
	const Method& method = spec.clientMethods[packet.methodId];

	packet.values.reserve(method.args.size());
	for (const ArgType& type : method.args)
	{
		if (std::optional<ArgValue> value = ParseValue(data, type))
		{
			packet.values.emplace_back(value.value());
		}
		else
		{
			LOG_ERROR("Failed to parse value for EntityMethodPacket");
			return InvalidPacket{};
		}
	}

	return packet;
}

inline std::variant<EntityCreatePacket, InvalidPacket> ParseEntityCreatePacket(std::span<std::byte>& data, PacketParser& parser, float clock)
{
	EntityCreatePacket packet;
	packet.type = PacketBaseType::EntityCreate;
	packet.clock = clock;

	auto err = [data]() -> InvalidPacket
	{
		LOG_ERROR("Failed to parse EntityCreatePacket: {}", FormatBytes(data));
		return InvalidPacket{};
	};

	if (!TakeInto(data, packet.entityId))
		return err();
	if (!TakeInto(data, packet.entityType))
		return err();
	if (!TakeInto(data, packet.vehicleId))
		return err();
	if (!TakeInto(data, packet.spaceId))
		return err();
	if (!TakeInto(data, packet.position))
		return err();
	if (!TakeInto(data, packet.direction))
		return err();

	if (parser.entities.contains(packet.entityId))
	{
		LOG_ERROR("Entity {} already exists for EntityCreatePacket", packet.entityId);
		return InvalidPacket{};
	}

	uint32_t size;
	if (!TakeInto(data, size))
		return err();

	if (data.size() != size)
	{
		LOG_ERROR("Invalid payload size on EntityCreatePacket: {} != {}", data.size(), size);
		return InvalidPacket{};
	}

	uint8_t propertyCount = 0;
	if (!TakeInto(data, propertyCount))
	{
		LOG_ERROR("Failed to get property count for EntityCreatePacket");
		return InvalidPacket{};
	}

	const int specId = packet.entityType - 1;
	if (specId < 0 || specId >= parser.specs.size())
	{
		LOG_ERROR("Missing EntitySpec {} for EntityCreatePacket", specId);
		return InvalidPacket{};
	}
	EntitySpec& spec = parser.specs[specId];

	packet.properties.reserve(propertyCount);
	for (uint8_t i = 0; i < propertyCount; i++)
	{
		uint8_t propertyId;
		if (!TakeInto(data, propertyId))
		{
			return InvalidPacket{};
		}
		if (propertyId < spec.properties.size())
		{
			Property& prop = spec.properties[propertyId];
			if (std::optional<ArgValue> value = ParseValue(data, prop.type))
			{
				packet.properties.insert({ spec.name, value });
			}
			else
			{
				LOG_ERROR("Failed to parse value for EntityCreatePacket");
				return InvalidPacket{};
			}
		}
		else
		{
			LOG_ERROR("Failed to get propertyId for EntityCreatePacket");
			return InvalidPacket{};
		}
	}

	const std::vector<ArgValue> values{ packet.properties.begin(), packet.properties.end() };
	parser.entities.emplace(packet.entityId, Entity{ packet.entityType, values });


	LOG_TRACE("Creating Entity {} with {} properties", packet.entityId, packet.properties.size());
	return packet;
}

inline std::variant<EntityPropertyPacket, InvalidPacket> ParseEntityPropertyPacket(std::span<std::byte>& data, const PacketParser& parser, float clock)
{
	EntityPropertyPacket packet;
	packet.clock = clock;
	packet.type = PacketBaseType::EntityProperty;

	auto err = [data]() -> InvalidPacket
	{
		LOG_ERROR("Failed to parse EntityPropertyPacket: {}", FormatBytes(data));
		return InvalidPacket{};
	};

	if (!TakeInto(data, packet.entityId))
		return err();
	if (!TakeInto(data, packet.methodId))
		return err();

	uint32_t size;
	if (!TakeInto(data, size))
		return err();

	if (data.size() != size)
	{
		LOG_ERROR("Invalid payload size on EntityPropertyPacket: {} != {}", data.size(), size);
		return InvalidPacket{};
	}

	if (!parser.entities.contains(packet.entityId))
	{
		LOG_ERROR("Entity {} does not exist for EntityPropertyPacket", packet.entityId);
		return InvalidPacket{};
	}
	const uint16_t entityType = parser.entities.at(packet.entityId).type;

	const int specId = entityType - 1;
	if (specId < 0 || specId >= parser.specs.size())
	{
		LOG_ERROR("Missing EntitySpec {} for EntityPropertyPacket", specId);
		return InvalidPacket{};
	}
	const EntitySpec& spec = parser.specs[specId];

	if (packet.methodId >= spec.properties.size())
	{
		LOG_ERROR("Invalid methodId {} for EntityPropertyPacket", packet.methodId);
		return InvalidPacket{};
	}
	const Property& property = spec.properties[packet.methodId];

	if (std::optional<ArgValue> value = ParseValue(data, property.type))
	{
		packet.value = value;
	}
	else
	{
		LOG_ERROR("Failed to parse value for EntityPropertyPacket");
		return InvalidPacket{};
	}

	return packet;
}

inline std::variant<BasePlayerCreatePacket, InvalidPacket> ParseBasePlayerCreatePacket(std::span<std::byte>& data, PacketParser& parser, float clock)
{
	BasePlayerCreatePacket packet;
	packet.clock = clock;
	packet.type = PacketBaseType::BasePlayerCreate;

	auto err = [data]() -> InvalidPacket
	{
		LOG_ERROR("Failed to parse CellPlayerCreatePacket: {}", FormatBytes(data));
		return InvalidPacket{};
	};

	if (!TakeInto(data, packet.entityId))
		return InvalidPacket{};
	if (!TakeInto(data, packet.entityType))
		return InvalidPacket{};

	const int specId = packet.entityType - 1;
	if (specId < 0 || specId >= parser.specs.size())
	{
		LOG_ERROR("Missing EntitySpec {} for BasePlayerCreatePacket", specId);
		return InvalidPacket{};
	}
	const EntitySpec& spec = parser.specs[specId];

	parser.entities.emplace(packet.entityId, Entity{ packet.entityType, {} });  // TODO: parse the state

	std::span<std::byte> state = Take(data, data.size());
	packet.data = { state.begin(), state.end() };

	return packet;
}

inline std::variant<CellPlayerCreatePacket, InvalidPacket> ParseCellPlayerCreatePacket(std::span<std::byte>& data, const PacketParser& parser, float clock)
{
	CellPlayerCreatePacket packet;
	packet.type = PacketBaseType::CellPlayerCreate;
	packet.clock = clock;

	auto err = [data]() -> InvalidPacket
	{
		LOG_ERROR("Failed to parse CellPlayerCreatePacket: {}", FormatBytes(data));
		return InvalidPacket{};
	};

	if (!TakeInto(data, packet.entityId))
		return err();
	if (!TakeInto(data, packet.spaceId))
		return err();
	if (!TakeInto(data, packet.vehicleId))
		return err();
	if (!TakeInto(data, packet.position))
		return err();
	if (!TakeInto(data, packet.direction))
		return err();

	uint32_t size;
	if (!TakeInto(data, size))
		return err();

	if (data.size() != size)
	{
		LOG_ERROR("Invalid payload size on CellPlayerCreatePacket: {} != {}", data.size(), size);
		return InvalidPacket{};
	}

	if (!parser.entities.contains(packet.entityId))
	{
		LOG_ERROR("Entity {} does not exist for CellPlayerCreatePacket", packet.entityId);
		return InvalidPacket{};
	}

	const uint16_t entityType = parser.entities.at(packet.entityId).type;

	const int specId = entityType - 1;
	if (specId < 0 || specId >= parser.specs.size())
	{
		LOG_ERROR("Missing EntitySpec {} for CellPlayerCreatePacket", specId);
		return InvalidPacket{};
	}
	const EntitySpec& spec = parser.specs[specId];

	packet.values.reserve(spec.internalProperties.size());
	for (const Property& property : spec.internalProperties)
	{
		if (std::optional<ArgValue> value = ParseValue(data, property.type))
		{
			packet.values.emplace_back(value.value());
		}
		else
		{
			LOG_ERROR("Failed to parse value for CellPlayerCreatePacket");
			return InvalidPacket{};
		}
	}

	if (!data.empty())
	{
		LOG_WARN("CellPlayerCreatePacket had {} bytes remaining after parsing", data.size());
	}

	return packet;
}

inline std::variant<EntityControlPacket, InvalidPacket> ParseEntityControlPacket(std::span<std::byte>& data, float clock)
{
	EntityControlPacket packet;
	packet.type = PacketBaseType::EntityControl;
	packet.clock = clock;

	auto err = [data]() -> InvalidPacket
	{
		LOG_ERROR("Failed to parse EntityControlPacket: {}", FormatBytes(data));
		return InvalidPacket{};
	};

	if (!TakeInto(data, packet.entityId))
		return err();
	if (!TakeInto(data, packet.isControlled))
		return err();

	if (!data.empty())
	{
		LOG_WARN("EntityControlPacket had {} bytes remaining after parsing", data.size());
	}

	return packet;
}

inline std::variant<EntityEnterPacket, InvalidPacket> ParseEntityEnterPacket(std::span<std::byte>& data, float clock)
{
	EntityEnterPacket packet;
	packet.type = PacketBaseType::EntityEnter;
	packet.clock = clock;

	auto err = [data]() -> InvalidPacket
	{
		LOG_ERROR("Failed to parse EntityEnterPacket: {}", FormatBytes(data));
		return InvalidPacket{};
	};

	if (!TakeInto(data, packet.entityId))
		return err();
	if (!TakeInto(data, packet.spaceId))
		return err();
	if (!TakeInto(data, packet.vehicleId))
		return err();

	if (!data.empty())
	{
		LOG_WARN("EntityEnterPacket had {} bytes remaining after parsing", data.size());
	}

	return packet;
}

inline std::variant<EntityLeavePacket, InvalidPacket> ParseEntityLeavePacket(std::span<std::byte>& data, float clock)
{
	EntityLeavePacket packet;
	packet.type = PacketBaseType::EntityLeave;
	packet.clock = clock;

	if (!TakeInto(data, packet.entityId))
	{
		LOG_ERROR("Failed to parse EntityLeavePacket: {}", FormatBytes(data));
		return InvalidPacket{};
	}

	if (!data.empty())
	{
		LOG_WARN("EntityLeavePacket had {} bytes remaining after parsing", data.size());
	}

	return packet;
}

inline std::variant<NestedPropertyUpdatePacket, InvalidPacket> ParseNestedPropertyUpdatePacket(std::span<std::byte>& data, PacketParser& parser, float clock)
{
	NestedPropertyUpdatePacket packet;
	packet.type = PacketBaseType::NestedPropertyUpdate;
	packet.clock = clock;

	auto err = [data]() -> InvalidPacket
	{
		LOG_ERROR("Failed to parse NestedPropertyUpdatePacket: {}", FormatBytes(data));
		return InvalidPacket{};
	};

	if (!data.empty())
	{
		// LOG_WARN("NestedPropertyUpdatePacket had {} bytes remaining after parsing", data.size());
	}

	return packet;
}

inline std::variant<PlayerOrientationPacket, InvalidPacket> ParsePlayerOrientationPacket(std::span<std::byte>& data, float clock)
{
	PlayerOrientationPacket packet;
	packet.type = PacketBaseType::PlayerOrientation;
	packet.clock = clock;

	auto err = [data]() -> InvalidPacket
	{
		LOG_ERROR("Failed to parse PlayerOrientationPacket: {}", FormatBytes(data));
		return InvalidPacket{};
	};

	if (!TakeInto(data, packet.pid))
		return err();
	if (!TakeInto(data, packet.parentId))
		return err();
	if (!TakeInto(data, packet.position))
		return err();
	if (!TakeInto(data, packet.direction))
		return err();

	if (!data.empty())
	{
		LOG_WARN("PlayerOrientationPacket had {} bytes remaining after parsing", data.size());
	}

	return packet;
}

inline std::variant<PlayerPositionPacket, InvalidPacket> ParsePlayerPositionPacketPacket(std::span<std::byte>& data, float clock)
{
	PlayerPositionPacket packet;
	packet.type = PacketBaseType::PlayerOrientation;
	packet.clock = clock;

	auto err = [data]() -> InvalidPacket
	{
		LOG_ERROR("Failed to parse PlayerPositionPacket - remaining: {}", FormatBytes(data));
		return InvalidPacket{};
	};

	if (!TakeInto(data, packet.entityId))
		return err();
	if (!TakeInto(data, packet.vehicleId))
		return err();
	if (!TakeInto(data, packet.position))
		return err();
	if (!TakeInto(data, packet.positionError))
		return err();
	if (!TakeInto(data, packet.direction))
		return err();
	if (!TakeInto(data, packet.isError))
		return err();

	if (!data.empty())
	{
		LOG_WARN("PlayerPositionPacket had {} bytes remaining after parsing", data.size());
	}

	return packet;
}

}  // namespace PotatoAlert::ReplayParser