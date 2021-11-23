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

template<typename R, typename T>
static R VariantCast(T&& t)
{
	return std::visit([]<typename T0>(T0&& val) -> R
	{
		return { std::forward<T0>(val) };
	}, std::forward<T>(t));
}

PacketType rp::ParsePacket(std::span<std::byte>& data, PacketParser& parser)
{
	uint32_t size;
	if (!TakeInto(data, size))
		return InvalidPacket{};
	uint32_t type;
	if (!TakeInto(data, type))
		return InvalidPacket{};
	float clock;
	if (!TakeInto(data, clock))
		return InvalidPacket{};

	auto raw = Take(data, size);

#ifndef NDEBUG
	size_t rawSize = raw.size();
#endif

	switch (type)
	{
		case static_cast<uint32_t>(PacketBaseType::Version):
			return VariantCast<PacketType>(ParseVersionPacket(raw, clock));
		case static_cast<uint32_t>(PacketBaseType::BasePlayerCreate):
			return VariantCast<PacketType>(ParseBasePlayerCreatePacket(raw, parser, clock));
		case static_cast<uint32_t>(PacketBaseType::CellPlayerCreate):
			return VariantCast<PacketType>(ParseCellPlayerCreatePacket(raw, parser, clock));
		case static_cast<uint32_t>(PacketBaseType::EntityControl):
			return VariantCast<PacketType>(ParseEntityControlPacket(raw, clock));
		case static_cast<uint32_t>(PacketBaseType::EntityEnter):
			return VariantCast<PacketType>(ParseEntityEnterPacket(raw, clock));
		case static_cast<uint32_t>(PacketBaseType::EntityLeave):
			return VariantCast<PacketType>(ParseEntityLeavePacket(raw, clock));
		case static_cast<uint32_t>(PacketBaseType::EntityCreate):
			return VariantCast<PacketType>(ParseEntityCreatePacket(raw, parser, clock));
		case static_cast<uint32_t>(PacketBaseType::EntityProperty):
			return VariantCast<PacketType>(ParseEntityPropertyPacket(raw, parser, clock));
		case static_cast<uint32_t>(PacketBaseType::EntityMethod):
			return VariantCast<PacketType>(ParseEntityMethodPacket(raw, parser, clock));
		case static_cast<uint32_t>(PacketBaseType::PlayerPosition):
			return VariantCast<PacketType>(ParsePlayerPositionPacketPacket(raw, clock));
		case static_cast<uint32_t>(PacketBaseType::PlayerEntity):
			return VariantCast<PacketType>(ParsePlayerEntityPacket(raw, clock));
		case static_cast<uint32_t>(PacketBaseType::NestedPropertyUpdate):
			return VariantCast<PacketType>(ParseNestedPropertyUpdatePacket(raw, parser, clock));
		case static_cast<uint32_t>(PacketBaseType::PlayerOrientation):
			return VariantCast<PacketType>(ParsePlayerOrientationPacket(raw, clock));
		case static_cast<uint32_t>(PacketBaseType::Camera):
			return VariantCast<PacketType>(ParseCameraPacket(raw, clock));
		case static_cast<uint32_t>(PacketBaseType::Map):
			return VariantCast<PacketType>(ParseMapPacket(raw, clock));
		default:  // unknown
			break;
#ifndef NDEBUG
		case 0xE:
		{
			// some sort of unique id for who is recording the replay
			uint64_t id;
			TakeInto(raw, id);
			// LOG_TRACE("0xE: {} {}", f1, f2);
			break;
		}
		case 0xF:
		{
			LOG_TRACE("0xF: {} bytes", raw.size());
			break;
		}
		case 0x10:
		{
			bool b1;
			TakeInto(raw, b1);
			LOG_TRACE("0x10: {} -> {} bytes", b1, rawSize);
			break;
		}
		case 0x13:
		{
			LOG_TRACE("0x13: {} bytes", rawSize);
			break;
		}
		case 0x18:
		{
			// somehow related to the camera
			Take(raw, 40);  // always 40 bytes of zeroes
			Vec3 unknown;  // always Vec3(-1.0f, -1.0f, -1.0f)
			TakeInto(raw, unknown);
			break;
		}
		case 0x25:
		{
			// 10 bytes
			uint32_t entityId;
			TakeInto(raw, entityId);
			LOG_TRACE("0x25: entityId {} -> {} bytes", entityId, rawSize);
			break;
		}
		case 0x26:
		{
			uint32_t unknown;
			TakeInto(raw, unknown);
			LOG_TRACE("0x26: {} -> {} bytes", unknown, rawSize);
			break;
		}
		case 0x29:
		{
			// 32 bytes
			LOG_TRACE("0x29: {} bytes", rawSize);
			break;
		}
		case 0x2E:
		{
			bool lockedTurrets;
			TakeInto(raw, lockedTurrets);
			LOG_TRACE("0x2E: lockedTurrets {} -> {} bytes", lockedTurrets, rawSize);
			break;
		}
		case 0x2F:
		{
			uint32_t u1, u2;
			TakeInto(raw, u1);
			TakeInto(raw, u2);
			uint32_t shipId;  // always enemy ship
			TakeInto(raw, shipId);
			LOG_TRACE("0x2F: u1 {} u2 {} shipId {} -> {} bytes", u1, u2, shipId, rawSize);
			break;
		}
		case 0x31:
		{
			assert(rawSize == 8);
			// 8 bytes, all zeroes
			uint32_t u1, u2;
			TakeInto(raw, u1);
			TakeInto(raw, u2);
			LOG_TRACE("0x31: {} {} -> {} bytes", u1, u2, rawSize);
			break;
		}
		case 0xFFFFFFFF:
		{
			LOG_TRACE("0xFFFFFFFF: {} bytes", rawSize);
			break;
		}
#endif
	}

	return UnknownPacket{};
}

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

	packet.values.reserve(propertyCount);
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
				packet.values[name] = value.value();
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

	const std::vector<ArgValue> values{ packet.values.begin(), packet.values.end() };
	parser.entities.emplace(packet.entityId, Entity{ packet.entityType, values });

	LOG_TRACE("Creating Entity {} with {} properties", packet.entityId, packet.values.size());
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
		return err();
	if (!TakeInto(data, packet.entityType))
		return err();

	const int specId = packet.entityType - 1;
	if (specId < 0 || specId >= parser.specs.size())
	{
		LOG_ERROR("Missing EntitySpec {} for BasePlayerCreatePacket", specId);
		return InvalidPacket{};
	}
	EntitySpec& spec = parser.specs[specId];

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
			packet.values[property.name] = value.value();
			// packet.values.emplace_back(value.value());
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

	if (!TakeInto(data, packet.entityId))
		return err();
	bool isSlice;
	if (!TakeInto(data, isSlice))
		return err();

	uint32_t size;
	if (!TakeInto(data, size))
		return err();

	if (data.size() != size)
	{
		LOG_ERROR("Invalid payload size on NestedPropertyUpdatePacket: {} != {}", data.size(), size);
		return InvalidPacket{};
	}

	Take(data, size);

	// TODO

	if (!data.empty())
	{
		LOG_WARN("NestedPropertyUpdatePacket had {} bytes remaining after parsing", data.size());
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
		LOG_ERROR("Failed to parse PlayerPositionPacket: {}", FormatBytes(data));
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
		LOG_ERROR("Failed to parse CameraPacket: {}", FormatBytes(data));
		return InvalidPacket{};
	};

	if (!TakeInto(data, packet.unknown))
		return err();
	if (!TakeInto(data, packet.unknown2))
		return err();
	if (!TakeInto(data, packet.absolutePosition))
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
		LOG_ERROR("Failed to parse MapPacket: {}", FormatBytes(data));
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
		LOG_ERROR("Failed to parse VersionPacket: {}", FormatBytes(data));
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

std::variant<PlayerEntityPacket, InvalidPacket> rp::ParsePlayerEntityPacket(std::span<std::byte>& data, float clock)
{
	PlayerEntityPacket packet;
	packet.type = PacketBaseType::PlayerEntity;
	packet.clock = clock;

	auto err = [data]() -> InvalidPacket
	{
		LOG_ERROR("Failed to parse PlayerEntityPacket: {}", FormatBytes(data));
		return InvalidPacket{};
	};

	if (!TakeInto(data, packet.entityId))
		return err();

	if (!data.empty())
	{
		LOG_WARN("PlayerEntityPacket had {} bytes remaining after parsing", data.size());
	}

	return packet;
}
