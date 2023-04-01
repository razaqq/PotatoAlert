// Copyright 2021 <github.com/razaqq>

#include "Core/Bytes.hpp"
#include "Core/Log.hpp"

#include "ReplayParser/BitReader.hpp"
#include "ReplayParser/PacketParser.hpp"

#include <optional>
#include <span>
#include <unordered_map>
#include <variant>
#include <vector>


using namespace PotatoAlert::ReplayParser;
namespace rp = PotatoAlert::ReplayParser;
using PotatoAlert::Core::FormatBytes;
using PotatoAlert::Core::Take;
using PotatoAlert::Core::TakeInto;
using PotatoAlert::Core::TakeString;

namespace {

template<typename R, typename T>
static R VariantCast(T&& t)
{
	return std::visit([]<typename T0>(T0&& val) -> R
	{
		return { std::forward<T0>(val) };
	}, std::forward<T>(t));
}

}  // namespace

PacketType rp::ParsePacket(std::span<const Byte>& data, PacketParser& parser)
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
		case static_cast<uint32_t>(PacketBaseType::EntityCreate):
			return VariantCast<PacketType>(ParseEntityCreatePacket(raw, parser, clock));
		case static_cast<uint32_t>(PacketBaseType::BasePlayerCreate):
			return VariantCast<PacketType>(ParseBasePlayerCreatePacket(raw, parser, clock)); 
		case static_cast<uint32_t>(PacketBaseType::CellPlayerCreate):
			return VariantCast<PacketType>(ParseCellPlayerCreatePacket(raw, parser, clock));
		case static_cast<uint32_t>(PacketBaseType::EntityMethod):
			return VariantCast<PacketType>(ParseEntityMethodPacket(raw, parser, clock)); 
		case static_cast<uint32_t>(PacketBaseType::EntityProperty):
			return VariantCast<PacketType>(ParseEntityPropertyPacket(raw, parser, clock)); 
		case static_cast<uint32_t>(PacketBaseType::NestedPropertyUpdate):
			return VariantCast<PacketType>(ParseNestedPropertyUpdatePacket(raw, parser, clock));

#ifndef NDEBUG
		case static_cast<uint32_t>(PacketBaseType::Version):
			return VariantCast<PacketType>(ParseVersionPacket(raw, parser, clock));
		case static_cast<uint32_t>(PacketBaseType::EntityControl):
			return VariantCast<PacketType>(ParseEntityControlPacket(raw, parser, clock));
		case static_cast<uint32_t>(PacketBaseType::EntityEnter):
			return VariantCast<PacketType>(ParseEntityEnterPacket(raw, parser, clock));
		case static_cast<uint32_t>(PacketBaseType::EntityLeave):
			return VariantCast<PacketType>(ParseEntityLeavePacket(raw, parser, clock));
		case static_cast<uint32_t>(PacketBaseType::PlayerPosition):
			return VariantCast<PacketType>(ParsePlayerPositionPacketPacket(raw, parser, clock));
		case static_cast<uint32_t>(PacketBaseType::PlayerEntity):
			return VariantCast<PacketType>(ParsePlayerEntityPacket(raw, parser, clock));
		case static_cast<uint32_t>(PacketBaseType::PlayerOrientation):
			return VariantCast<PacketType>(ParsePlayerOrientationPacket(raw, parser, clock));
		case static_cast<uint32_t>(PacketBaseType::Camera):
			return VariantCast<PacketType>(ParseCameraPacket(raw, parser, clock));
		case static_cast<uint32_t>(PacketBaseType::Map):
			return VariantCast<PacketType>(ParseMapPacket(raw, parser, clock));
		case static_cast<uint32_t>(PacketBaseType::CameraFreeLook):
			return VariantCast<PacketType>(ParseCameraFreeLookPacket(raw, parser, clock));
		case static_cast<uint32_t>(PacketBaseType::CameraMode):
			return VariantCast<PacketType>(ParseCameraModePacket(raw, parser, clock));
		case static_cast<uint32_t>(PacketBaseType::CruiseState):
			return VariantCast<PacketType>(ParseCruiseStatePacket(raw, parser, clock));
#endif  // NDEBUG

#if 0
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
#endif  // 0

		default:  // unknown
			break;
	}

	return UnknownPacket{};
}

std::variant<EntityMethodPacket, InvalidPacket> rp::ParseEntityMethodPacket(std::span<const Byte>& data, PacketParser& parser, float clock)
{
	EntityMethodPacket packet;
	packet.Type = PacketBaseType::EntityMethod;
	packet.Clock = clock;

	auto err = [data]() -> InvalidPacket
	{
		LOG_ERROR("Failed to parse EntityMethodPacket: {}", FormatBytes(data));
		return InvalidPacket{};
	};

	if (!TakeInto(data, packet.EntityId))
		return err();
	if (!TakeInto(data, packet.MethodId))
		return err();

	uint32_t size;
	if (!TakeInto(data, size))
		return err();

	if (data.size() != size)
	{
		LOG_ERROR("Invalid payload size on EntityMethodPacket: {} != {}", data.size(), size);
		return InvalidPacket{};
	}

	if (!parser.Entities.contains(packet.EntityId))
	{
		LOG_ERROR("Entity {} does not exist for EntityMethodPacket", packet.EntityId);
		return InvalidPacket{};
	}
	const uint16_t entityType = parser.Entities.at(packet.EntityId).Type;

	const int specId = entityType - 1;
	if (specId < 0 || specId >= parser.Specs.size())
	{
		LOG_ERROR("Missing EntitySpec {} for EntityMethodPacket", specId);
		return InvalidPacket{};
	}
	const EntitySpec& spec = parser.Specs[specId];

	if (packet.MethodId >= spec.ClientMethods.size())
	{
		LOG_ERROR("Invalid methodId {} for EntityMethodPacket", packet.MethodId);
		return InvalidPacket{};
	}
	const Method& method = spec.ClientMethods[packet.MethodId];
	packet.MethodName = method.Name;

	packet.Values.reserve(method.Args.size());
	for (const ArgType& argType : method.Args)
	{
		if (std::optional<ArgValue> value = ParseValue(data, argType))
		{
			packet.Values.emplace_back(value.value());
		}
		else
		{
			LOG_ERROR("Failed to parse value for EntityMethodPacket");
			return InvalidPacket{};
		}
	}

	parser.Callbacks.Invoke(packet);
	return packet;
}

std::variant<EntityCreatePacket, InvalidPacket> rp::ParseEntityCreatePacket(std::span<const Byte>& data, PacketParser& parser, float clock)
{
	EntityCreatePacket packet;
	packet.Type = PacketBaseType::EntityCreate;
	packet.Clock = clock;

	auto err = [data]() -> InvalidPacket
	{
		LOG_ERROR("Failed to parse EntityCreatePacket: {}", FormatBytes(data));
		return InvalidPacket{};
	};

	if (!TakeInto(data, packet.EntityId))
		return err();
	if (!TakeInto(data, packet.EntityType))
		return err();
	if (!TakeInto(data, packet.SpaceId))
		return err();
	if (!TakeInto(data, packet.VehicleId))
		return err();
	if (!TakeInto(data, packet.Position))
		return err();
	if (!TakeInto(data, packet.Rotation))
		return err();

	if (parser.Entities.contains(packet.EntityId))
	{
		// LOG_TRACE("Entity {} already exists for EntityCreatePacket", packet.EntityId);
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

	const int specId = packet.EntityType - 1;
	if (specId < 0 || specId >= parser.Specs.size())
	{
		LOG_ERROR("Missing EntitySpec {} for EntityCreatePacket", specId);
		return InvalidPacket{};
	}
	const EntitySpec& spec = parser.Specs[specId];

	packet.Values.reserve(propertyCount);

	std::unordered_map<std::string, ArgValue> clientPropertyValues;
	clientPropertyValues.reserve(propertyCount);

	for (uint8_t i = 0; i < propertyCount; i++)
	{
		uint8_t propertyId;
		if (!TakeInto(data, propertyId))
		{
			LOG_ERROR("Failed to get propertyId for EntityCreatePacket, not enough data");
			return InvalidPacket{};
		}
		if (propertyId < spec.ClientProperties.size())
		{
			const auto& [name, type, flag] = spec.ClientProperties[propertyId].get();

			if (std::optional<ArgValue> value = ParseValue(data, type))
			{
				packet.Values[name] = value.value();
				clientPropertyValues.insert_or_assign(name, value.value());
			}
			else
			{
				LOG_ERROR("Failed to parse value for EntityCreatePacket");
				return InvalidPacket{};
			}
		}
		else
		{
			LOG_ERROR("Failed to get propertyId for EntityCreatePacket, out of bounds");
			return InvalidPacket{};
		}
	}

	parser.Entities.insert_or_assign(packet.EntityId, Entity{ packet.EntityType, spec, {}, clientPropertyValues });

	parser.Callbacks.Invoke(packet);
	return packet;
}

std::variant<EntityPropertyPacket, InvalidPacket> rp::ParseEntityPropertyPacket(std::span<const Byte>& data, PacketParser& parser, float clock)
{
	EntityPropertyPacket packet;
	packet.Clock = clock;
	packet.Type = PacketBaseType::EntityProperty;

	auto err = [data]() -> InvalidPacket
	{
		LOG_ERROR("Failed to parse EntityPropertyPacket: {}", FormatBytes(data));
		return InvalidPacket{};
	};

	if (!TakeInto(data, packet.EntityId))
		return err();
	if (!TakeInto(data, packet.MethodId))
		return err();

	uint32_t size;
	if (!TakeInto(data, size))
		return err();

	if (data.size() != size)
	{
		LOG_ERROR("Invalid payload size on EntityPropertyPacket: {} != {}", data.size(), size);
		return InvalidPacket{};
	}

	if (!parser.Entities.contains(packet.EntityId))
	{
		LOG_ERROR("Entity {} does not exist for EntityPropertyPacket", packet.EntityId);
		return InvalidPacket{};
	}
	const uint16_t entityType = parser.Entities.at(packet.EntityId).Type;

	const int specId = entityType - 1;
	if (specId < 0 || specId >= parser.Specs.size())
	{
		LOG_ERROR("Missing EntitySpec {} for EntityPropertyPacket", specId);
		return InvalidPacket{};
	}
	const EntitySpec& spec = parser.Specs[specId];

	if (packet.MethodId >= spec.ClientProperties.size())
	{
		LOG_ERROR("Invalid methodId {} for EntityPropertyPacket", packet.MethodId);
		return InvalidPacket{};
	}
	const Property& property = spec.ClientProperties[packet.MethodId];
	packet.PropertyName = property.Name;

	if (!parser.Entities.contains(packet.EntityId))
	{
		LOG_ERROR("Entity {} does not exist for EntityPropertyPacket", packet.EntityId);
		return InvalidPacket{};
	}
	Entity& entity = parser.Entities.at(packet.EntityId);

	if (std::optional<ArgValue> value = ParseValue(data, property.Type))
	{
		packet.Value = value.value();
		entity.ClientPropertiesValues.insert_or_assign(property.Name, value.value());
	}
	else
	{
		LOG_ERROR("Failed to parse value for EntityPropertyPacket");
		return InvalidPacket{};
	}

	parser.Callbacks.Invoke(packet);
	return packet;
}

std::variant<BasePlayerCreatePacket, InvalidPacket> rp::ParseBasePlayerCreatePacket(std::span<const Byte>& data, PacketParser& parser, float clock)
{
	BasePlayerCreatePacket packet;
	packet.Clock = clock;
	packet.Type = PacketBaseType::BasePlayerCreate;

	auto err = [data]() -> InvalidPacket
	{
		LOG_ERROR("Failed to parse CellPlayerCreatePacket: {}", FormatBytes(data));
		return InvalidPacket{};
	};

	if (!TakeInto(data, packet.EntityId))
		return err();
	if (!TakeInto(data, packet.EntityType))
		return err();

	const int specId = packet.EntityType - 1;
	if (specId < 0 || specId >= parser.Specs.size())
	{
		LOG_ERROR("Missing EntitySpec {} for BasePlayerCreatePacket", specId);
		return InvalidPacket{};
	}
	const EntitySpec& spec = parser.Specs[specId];

	const size_t propertyCount = spec.BaseProperties.size();
	std::unordered_map<std::string, ArgValue> basePropertyValues;
	basePropertyValues.reserve(propertyCount);

	for (uint8_t i = 0; i < propertyCount; i++)
	{
		const auto& [name, type, flag] = spec.BaseProperties[i].get();

		if (std::optional<ArgValue> value = ParseValue(data, type))
		{
			basePropertyValues.emplace(name, value.value());
		}
		else
		{
			LOG_ERROR("Failed to parse value for EntityCreatePacket");
			return InvalidPacket{};
		}
	}

	parser.Entities.emplace(packet.EntityId, Entity{ packet.EntityType, spec, basePropertyValues, {} });  // TODO: parse the state

	std::span<const Byte> state = Take(data, data.size());
	packet.Data = { state.begin(), state.end() };

	parser.Callbacks.Invoke(packet);
	return packet;
}

std::variant<CellPlayerCreatePacket, InvalidPacket> rp::ParseCellPlayerCreatePacket(std::span<const Byte>& data, PacketParser& parser, float clock)
{
	CellPlayerCreatePacket packet;
	packet.Type = PacketBaseType::CellPlayerCreate;
	packet.Clock = clock;

	auto err = [data]() -> InvalidPacket
	{
		LOG_ERROR("Failed to parse CellPlayerCreatePacket: {}", FormatBytes(data));
		return InvalidPacket{};
	};

	if (!TakeInto(data, packet.EntityId))
		return err();
	if (!TakeInto(data, packet.SpaceId))
		return err();
	if (!TakeInto(data, packet.VehicleId))
		return err();
	if (!TakeInto(data, packet.Position))
		return err();
	if (!TakeInto(data, packet.Rotation))
		return err();

	uint32_t size;
	if (!TakeInto(data, size))
		return err();

	if (data.size() != size)
	{
		LOG_ERROR("Invalid payload size on CellPlayerCreatePacket: {} != {}", data.size(), size);
		return InvalidPacket{};
	}

	if (!parser.Entities.contains(packet.EntityId))
	{
		LOG_ERROR("Entity {} does not exist for CellPlayerCreatePacket", packet.EntityId);
		return InvalidPacket{};
	}

	const uint16_t entityType = parser.Entities.at(packet.EntityId).Type;

	const int specId = entityType - 1;
	if (specId < 0 || specId >= parser.Specs.size())
	{
		LOG_ERROR("Missing EntitySpec {} for CellPlayerCreatePacket", specId);
		return InvalidPacket{};
	}
	const EntitySpec& spec = parser.Specs[specId];

	if (!parser.Entities.contains(packet.EntityId))
	{
		LOG_WARN("CellPlayerCreatePacket created non-existing entity {}", packet.EntityId);
		parser.Entities.emplace(packet.EntityId, Entity{ entityType, spec, {} });
	}

	packet.Values.reserve(spec.ClientPropertiesInternal.size());
	for (auto property : spec.ClientPropertiesInternal)
	{
		if (std::optional<ArgValue> value = ParseValue(data, property.get().Type))
		{
			packet.Values[property.get().Name] = value.value();
			parser.Entities.at(packet.EntityId).ClientPropertiesValues.emplace(property.get().Name, value.value());
		}
		else
		{
			LOG_ERROR("Failed to parse value for CellPlayerCreatePacket");
			return InvalidPacket{};
		}
	}

	bool unknown;
	TakeInto(data, unknown);

	if (!data.empty())
	{
		LOG_WARN("CellPlayerCreatePacket had {} bytes remaining after parsing", data.size());
	}

	parser.Callbacks.Invoke(packet);
	return packet;
}

std::variant<EntityControlPacket, InvalidPacket> rp::ParseEntityControlPacket(std::span<const Byte>& data, const PacketParser& parser, float clock)
{
	EntityControlPacket packet;
	packet.Type = PacketBaseType::EntityControl;
	packet.Clock = clock;

	auto err = [data]() -> InvalidPacket
	{
		LOG_ERROR("Failed to parse EntityControlPacket: {}", FormatBytes(data));
		return InvalidPacket{};
	};

	if (!TakeInto(data, packet.EntityId))
		return err();
	if (!TakeInto(data, packet.IsControlled))
		return err();

	if (!data.empty())
	{
		LOG_WARN("EntityControlPacket had {} bytes remaining after parsing", data.size());
	}

	parser.Callbacks.Invoke(packet);
	return packet;
}

std::variant<EntityEnterPacket, InvalidPacket> rp::ParseEntityEnterPacket(std::span<const Byte>& data, const PacketParser& parser, float clock)
{
	EntityEnterPacket packet;
	packet.Type = PacketBaseType::EntityEnter;
	packet.Clock = clock;

	auto err = [data]() -> InvalidPacket
	{
		LOG_ERROR("Failed to parse EntityEnterPacket: {}", FormatBytes(data));
		return InvalidPacket{};
	};

	if (!TakeInto(data, packet.EntityId))
		return err();
	if (!TakeInto(data, packet.SpaceId))
		return err();
	if (!TakeInto(data, packet.VehicleId))
		return err();

	if (!data.empty())
	{
		LOG_WARN("EntityEnterPacket had {} bytes remaining after parsing", data.size());
	}

	parser.Callbacks.Invoke(packet);
	return packet;
}

std::variant<EntityLeavePacket, InvalidPacket> rp::ParseEntityLeavePacket(std::span<const Byte>& data, const PacketParser& parser, float clock)
{
	EntityLeavePacket packet;
	packet.Type = PacketBaseType::EntityLeave;
	packet.Clock = clock;

	if (!TakeInto(data, packet.EntityId))
	{
		LOG_ERROR("Failed to parse EntityLeavePacket: {}", FormatBytes(data));
		return InvalidPacket{};
	}

	if (!data.empty())
	{
		LOG_WARN("EntityLeavePacket had {} bytes remaining after parsing", data.size());
	}

	parser.Callbacks.Invoke(packet);
	return packet;
}

std::variant<NestedPropertyUpdatePacket, InvalidPacket> rp::ParseNestedPropertyUpdatePacket(std::span<const Byte>& data, PacketParser& parser, float clock)
{
	NestedPropertyUpdatePacket packet;
	packet.Type = PacketBaseType::NestedPropertyUpdate;
	packet.Clock = clock;

	auto err = [data]() -> InvalidPacket
	{
		LOG_ERROR("Failed to parse NestedPropertyUpdatePacket: {}", FormatBytes(data));
		return InvalidPacket{};
	};

	if (!TakeInto(data, packet.EntityId))
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

	std::span<const Byte> payload = Take(data, size);

	if (!parser.Entities.contains(packet.EntityId))
	{
		LOG_ERROR("Entity {} does not exist for EntityPropertyPacket", packet.EntityId);
		return InvalidPacket{};
	}
	packet.Entity = &parser.Entities.at(packet.EntityId);
	const EntitySpec& spec = packet.Entity->Spec;

	BitReader bitReader(payload);
	const int cont = bitReader.Get(1);
#ifndef NDEBUG
	assert(cont == 1);
#else
	if (cont != 1)
	{
		LOG_ERROR("Invalid first bit {:#04x} in NestedPropertyUpdatePacket payload", cont);
		return InvalidPacket{};
	}
#endif

	const int propIndex = bitReader.Get(BitReader::BitsRequired(static_cast<int>(spec.ClientProperties.size())));
	if (propIndex >= spec.ClientProperties.size())
	{
		LOG_ERROR("Property index out of range ({}) for spec in NestedPropertyUpdatePacket", propIndex);
		return InvalidPacket{};
	}

	const Property& prop = spec.ClientProperties[propIndex].get();

	packet.PropertyIndex = propIndex;
	packet.PropertyName = prop.Name;

	if (!packet.Entity->ClientPropertiesValues.contains(prop.Name))
	{
		LOG_ERROR("Entity is missing property value for '{}' in NestedPropertyUpdatePacket", prop.Name);
		return InvalidPacket{};
	}

	ReplayResult<PropertyNesting> nesting = GetNestedPropertyPath(isSlice, prop.Type, &packet.Entity->ClientPropertiesValues[prop.Name], bitReader);
	if (!nesting)
	{
		LOG_ERROR(nesting.error());
		return InvalidPacket{};
	}
	packet.Nesting = nesting.value();

	if (!data.empty())
	{
		LOG_WARN("NestedPropertyUpdatePacket had {} bytes remaining after parsing", data.size());
	}

	parser.Callbacks.Invoke(packet);
	return packet;
}

std::variant<PlayerOrientationPacket, InvalidPacket> rp::ParsePlayerOrientationPacket(std::span<const Byte>& data, const PacketParser& parser, float clock)
{
	PlayerOrientationPacket packet;
	packet.Type = PacketBaseType::PlayerOrientation;
	packet.Clock = clock;

	auto err = [data]() -> InvalidPacket
	{
		LOG_ERROR("Failed to parse PlayerOrientationPacket: {}", FormatBytes(data));
		return InvalidPacket{};
	};

	if (!TakeInto(data, packet.Pid))
		return err();
	if (!TakeInto(data, packet.ParentId))
		return err();
	if (!TakeInto(data, packet.Position))
		return err();
	if (!TakeInto(data, packet.Rotation))
		return err();

	if (!data.empty())
	{
		LOG_WARN("PlayerOrientationPacket had {} bytes remaining after parsing", data.size());
	}

	parser.Callbacks.Invoke(packet);
	return packet;
}

std::variant<PlayerPositionPacket, InvalidPacket> rp::ParsePlayerPositionPacketPacket(std::span<const Byte>& data, const PacketParser& parser, float clock)
{
	PlayerPositionPacket packet;
	packet.Type = PacketBaseType::PlayerOrientation;
	packet.Clock = clock;

	auto err = [data]() -> InvalidPacket
	{
		LOG_ERROR("Failed to parse PlayerPositionPacket: {}", FormatBytes(data));
		return InvalidPacket{};
	};

	if (!TakeInto(data, packet.EntityId))
		return err();
	if (!TakeInto(data, packet.VehicleId))
		return err();
	if (!TakeInto(data, packet.Position))
		return err();
	if (!TakeInto(data, packet.PositionError))
		return err();
	if (!TakeInto(data, packet.Rotation))
		return err();
	if (!TakeInto(data, packet.IsError))
		return err();

	if (!data.empty())
	{
		LOG_WARN("PlayerPositionPacket had {} bytes remaining after parsing", data.size());
	}

	parser.Callbacks.Invoke(packet);
	return packet;
}

std::variant<CameraPacket, InvalidPacket> rp::ParseCameraPacket(std::span<const Byte>& data, const PacketParser& parser, float clock)
{
	CameraPacket packet;
	packet.Type = PacketBaseType::Camera;
	packet.Clock = clock;

	auto err = [data]() -> InvalidPacket
	{
		LOG_ERROR("Failed to parse CameraPacket: {}", FormatBytes(data));
		return InvalidPacket{};
	};

	if (!TakeInto(data, packet.Unknown))
		return err();
	if (!TakeInto(data, packet.Unknown2))
		return err();
	if (!TakeInto(data, packet.AbsolutePosition))
		return err();
	if (!TakeInto(data, packet.Fov))
		return err();
	if (!TakeInto(data, packet.Position))
		return err();
	if (!TakeInto(data, packet.Rotation))
		return err();

	// newer versions seem to have an additional 4 bytes, float
	if (data.size() >= sizeof(packet.Unknown3))
	{
		if (!TakeInto(data, packet.Unknown3))
			return err();
	}

	if (!data.empty())
	{
		LOG_WARN("CameraPacket had {} bytes remaining after parsing", data.size());
	}

	parser.Callbacks.Invoke(packet);
	return packet;
}

std::variant<MapPacket, InvalidPacket> rp::ParseMapPacket(std::span<const Byte>& data, const PacketParser& parser, float clock)
{
	MapPacket packet;
	packet.Type = PacketBaseType::Map;
	packet.Clock = clock;

	auto err = [data]() -> InvalidPacket
	{
		LOG_ERROR("Failed to parse MapPacket: {}", FormatBytes(data));
		return InvalidPacket{};
	};

	if (!TakeInto(data, packet.SpaceId))
		return err();
	if (!TakeInto(data, packet.ArenaId))
		return err();
	if (!TakeInto(data, packet.Unknown1))
		return err();
	if (!TakeInto(data, packet.Unknown2))
		return err();

	Take(data, 128);

	uint32_t stringSize;
	if (!TakeInto(data, stringSize))
		return err();
	if (!TakeString(data, packet.Name, stringSize))
		return err();
	if (!TakeInto(data, packet.Matrix))
		return err();
	if (!TakeInto(data, packet.Unknown3))
		return err();

	if (!data.empty())
	{
		LOG_WARN("MapPacket had {} bytes remaining after parsing", data.size());
	}

	parser.Callbacks.Invoke(packet);
	return packet;
}

std::variant<VersionPacket, InvalidPacket> rp::ParseVersionPacket(std::span<const Byte>& data, const PacketParser& parser, float clock)
{
	VersionPacket packet;
	packet.Type = PacketBaseType::Version;
	packet.Clock = clock;

	auto err = [data]() -> InvalidPacket
	{
		LOG_ERROR("Failed to parse VersionPacket: {}", FormatBytes(data));
		return InvalidPacket{};
	};

	uint32_t stringSize;
	if (!TakeInto(data, stringSize))
		return err();

	if (!TakeString(data, packet.Version, stringSize))
		return err();

	if (!data.empty())
	{
		LOG_WARN("VersionPacket had {} bytes remaining after parsing", data.size());
	}

	parser.Callbacks.Invoke(packet);
	return packet;
}

std::variant<PlayerEntityPacket, InvalidPacket> rp::ParsePlayerEntityPacket(std::span<const Byte>& data, const PacketParser& parser, float clock)
{
	PlayerEntityPacket packet;
	packet.Type = PacketBaseType::PlayerEntity;
	packet.Clock = clock;

	auto err = [data]() -> InvalidPacket
	{
		LOG_ERROR("Failed to parse PlayerEntityPacket: {}", FormatBytes(data));
		return InvalidPacket{};
	};

	if (!TakeInto(data, packet.EntityId))
		return err();

	if (!data.empty())
	{
		LOG_WARN("PlayerEntityPacket had {} bytes remaining after parsing", data.size());
	}

	parser.Callbacks.Invoke(packet);
	return packet;
}

std::variant<CameraModePacket, InvalidPacket> rp::ParseCameraModePacket(std::span<const Byte>& data, const PacketParser& parser, float clock)
{
	CameraModePacket packet;
	packet.Type = PacketBaseType::CameraMode;
	packet.Clock = clock;

	auto err = [data]() -> InvalidPacket
	{
		LOG_ERROR("Failed to parse CameraModePacket: {}", FormatBytes(data));
		return InvalidPacket{};
	};

	if (!TakeInto(data, packet.Mode))
		return err();

	if (!data.empty())
	{
		LOG_WARN("CameraModePacket had {} bytes remaining after parsing", data.size());
	}

	parser.Callbacks.Invoke(packet);
	return packet;
}

std::variant<CruiseStatePacket, InvalidPacket> rp::ParseCruiseStatePacket(std::span<const Byte>& data, const PacketParser& parser, float clock)
{
	CruiseStatePacket packet;
	packet.Type = PacketBaseType::CruiseState;
	packet.Clock = clock;

	auto err = [data]() -> InvalidPacket
	{
		LOG_ERROR("Failed to parse CruiseStatePacket: {}", FormatBytes(data));
		return InvalidPacket{};
	};
	
	if (!TakeInto(data, packet.Key))
		return err();

	if (!TakeInto(data, packet.Value))
		return err();

	if (!data.empty())
	{
		LOG_WARN("CruiseStatePacket had {} bytes remaining after parsing", data.size());
	}

	parser.Callbacks.Invoke(packet);
	return packet;
}

std::variant<CameraFreeLookPacket, InvalidPacket> rp::ParseCameraFreeLookPacket(std::span<const Byte>& data, const PacketParser& parser, float clock)
{
	CameraFreeLookPacket packet;
	packet.Type = PacketBaseType::CameraFreeLook;
	packet.Clock = clock;

	auto err = [data]() -> InvalidPacket
	{
		LOG_ERROR("Failed to parse CameraFreeLookPacket: {}", FormatBytes(data));
		return InvalidPacket{};
	};
	
	if (!TakeInto(data, packet.Locked))
		return err();

	if (!data.empty())
	{
		LOG_WARN("CameraFreeLookPacket had {} bytes remaining after parsing", data.size());
	}

	parser.Callbacks.Invoke(packet);
	return packet;
}
