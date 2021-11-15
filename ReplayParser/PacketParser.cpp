// Copyright 2021 <github.com/razaqq>

#include "PacketParser.hpp"

#include "Bytes.hpp"
#include "Log.hpp"

#include <optional>
#include <span>
#include <unordered_map>
#include <variant>
#include <vector>


using namespace PotatoAlert::ReplayParser;
namespace rp = PotatoAlert::ReplayParser;

std::variant<EntityMethodPacket, InvalidPacket> rp::ParseEntityMethodPacket(std::span<std::byte>& data, PacketParser& parser, float clock)
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
	const EntitySpec& spec = parser.specs[specId];

	if (packet.methodId >= spec.clientMethods.size())
	{
		LOG_ERROR("Invalid methodId {} for EntityMethodPacket", packet.methodId);
		return InvalidPacket{};
	}
	const Method& method = spec.clientMethods[packet.methodId];
	packet.methodName = method.name;

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

std::variant<EntityCreatePacket, InvalidPacket> rp::ParseEntityCreatePacket(std::span<std::byte>& data, PacketParser& parser, float clock)
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
	if (!TakeInto(data, packet.rotation))
		return err();

	if (parser.entities.contains(packet.entityId))
	{
		// LOG_ERROR("Entity {} already exists for EntityCreatePacket", packet.entityId);
		// return InvalidPacket{};
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
	const EntitySpec& spec = parser.specs[specId];

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
			const auto& [name, type, flag] = spec.properties[propertyId];

			if (std::optional<ArgValue> value = ParseValue(data, type))
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

std::variant<EntityPropertyPacket, InvalidPacket> rp::ParseEntityPropertyPacket(std::span<std::byte>& data, const PacketParser& parser, float clock)
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

std::variant<BasePlayerCreatePacket, InvalidPacket> rp::ParseBasePlayerCreatePacket(std::span<std::byte>& data, PacketParser& parser, float clock)
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

std::variant<CellPlayerCreatePacket, InvalidPacket> rp::ParseCellPlayerCreatePacket(std::span<std::byte>& data, const PacketParser& parser, float clock)
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
	if (!TakeInto(data, packet.rotation))
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

std::variant<EntityControlPacket, InvalidPacket> rp::ParseEntityControlPacket(std::span<std::byte>& data, float clock)
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

std::variant<EntityEnterPacket, InvalidPacket> rp::ParseEntityEnterPacket(std::span<std::byte>& data, float clock)
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

std::variant<EntityLeavePacket, InvalidPacket> rp::ParseEntityLeavePacket(std::span<std::byte>& data, float clock)
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

std::variant<NestedPropertyUpdatePacket, InvalidPacket> rp::ParseNestedPropertyUpdatePacket(std::span<std::byte>& data, PacketParser& parser, float clock)
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

std::variant<PlayerOrientationPacket, InvalidPacket> rp::ParsePlayerOrientationPacket(std::span<std::byte>& data, float clock)
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
	if (!TakeInto(data, packet.rotation))
		return err();

	if (!data.empty())
	{
		LOG_WARN("PlayerOrientationPacket had {} bytes remaining after parsing", data.size());
	}

	return packet;
}

std::variant<PlayerPositionPacket, InvalidPacket> rp::ParsePlayerPositionPacketPacket(std::span<std::byte>& data, float clock)
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
	if (!TakeInto(data, packet.rotation))
		return err();
	if (!TakeInto(data, packet.isError))
		return err();

	if (!data.empty())
	{
		LOG_WARN("PlayerPositionPacket had {} bytes remaining after parsing", data.size());
	}

	return packet;
}

std::variant<CameraPacket, InvalidPacket> rp::ParseCameraPacket(std::span<std::byte>& data, float clock)
{
	CameraPacket packet;
	packet.type = PacketBaseType::Camera;
	packet.clock = clock;

	auto err = [data]() -> InvalidPacket
	{
		LOG_ERROR("Failed to parse CameraPacket - remaining: {}", FormatBytes(data));
		return InvalidPacket{};
	};

	if (!TakeInto(data, packet.unknown))
		return err();
	if (!TakeInto(data, packet.unknown2))
		return err();
	if (!TakeInto(data, packet.unknown3))
		return err();
	if (!TakeInto(data, packet.fov))
		return err();
	if (!TakeInto(data, packet.position))
		return err();
	if (!TakeInto(data, packet.rotation))
		return err();

	if (!data.empty())
	{
		LOG_WARN("CameraPacket had {} bytes remaining after parsing", data.size());
	}

	return packet;
}

std::variant<MapPacket, InvalidPacket> rp::ParseMapPacket(std::span<std::byte>& data, float clock)
{
	MapPacket packet;
	packet.type = PacketBaseType::Map;
	packet.clock = clock;

	auto err = [data]() -> InvalidPacket
	{
		LOG_ERROR("Failed to parse MapPacket - remaining: {}", FormatBytes(data));
		return InvalidPacket{};
	};

	if (!TakeInto(data, packet.spaceId))
		return err();
	if (!TakeInto(data, packet.arenaId))
		return err();
	if (!TakeInto(data, packet.unknown1))
		return err();
	if (!TakeInto(data, packet.unknown2))
		return err();

	Take(data, 128);

	uint32_t stringSize;
	if (!TakeInto(data, stringSize))
		return err();
	if (!TakeString(data, packet.name, stringSize))
		return err();
	if (!TakeInto(data, packet.matrix))
		return err();
	if (!TakeInto(data, packet.unknown3))
		return err();

	if (!data.empty())
	{
		LOG_WARN("MapPacket had {} bytes remaining after parsing", data.size());
	}

	return packet;
}

std::variant<VersionPacket, InvalidPacket> rp::ParseVersionPacket(std::span<std::byte>& data, float clock)
{
	VersionPacket packet;
	packet.type = PacketBaseType::Version;
	packet.clock = clock;

	auto err = [data]() -> InvalidPacket
	{
		LOG_ERROR("Failed to parse VersionPacket - remaining: {}", FormatBytes(data));
		return InvalidPacket{};
	};

	uint32_t stringSize;
	if (!TakeInto(data, stringSize))
		return err();

	if (!TakeString(data, packet.version, stringSize))
		return err();

	if (!data.empty())
	{
		LOG_WARN("VersionPacket had {} bytes remaining after parsing", data.size());
	}

	return packet;
}
