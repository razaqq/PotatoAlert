// Copyright 2021 <github.com/razaqq>

#include "Core/Bytes.hpp"
#include "Core/Log.hpp"

#include "ReplayParser/BitReader.hpp"
#include "ReplayParser/PacketParser.hpp"
#include "ReplayParser/Result.hpp"

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

ReplayResult<PacketType> rp::ParsePacket(std::span<const Byte>& data, PacketParser& parser)
{
	uint32_t size;
	if (!TakeInto(data, size))
		return PA_REPLAY_ERROR("Packet had invalid size {}", data.size());
	uint32_t type;
	if (!TakeInto(data, type))
		return PA_REPLAY_ERROR("Packet had invalid size {}", data.size());
	float clock;
	if (!TakeInto(data, clock))
		return PA_REPLAY_ERROR("Packet had invalid size {}", data.size());

	auto raw = Take(data, size);

#ifndef NDEBUG
	size_t rawSize = raw.size();
#endif

	switch (static_cast<PacketBaseType>(type))
	{
		case PacketBaseType::EntityCreate:
			return ParseEntityCreatePacket(raw, parser, clock);
		case PacketBaseType::BasePlayerCreate:
			return ParseBasePlayerCreatePacket(raw, parser, clock);
		case PacketBaseType::CellPlayerCreate:
			return ParseCellPlayerCreatePacket(raw, parser, clock);
		case PacketBaseType::EntityMethod:
			return ParseEntityMethodPacket(raw, parser, clock);
		case PacketBaseType::EntityProperty:
			return ParseEntityPropertyPacket(raw, parser, clock);
		case PacketBaseType::NestedPropertyUpdate:
			return ParseNestedPropertyUpdatePacket(raw, parser, clock);
		case PacketBaseType::PlayerPosition:
			return ParsePlayerPositionPacketPacket(raw, parser, clock);
		case PacketBaseType::PlayerOrientation:
			return ParsePlayerOrientationPacket(raw, parser, clock);

#ifndef NDEBUG
		case PacketBaseType::Version:
			return ParseVersionPacket(raw, parser, clock);
		case PacketBaseType::EntityControl:
			return ParseEntityControlPacket(raw, parser, clock);
		case PacketBaseType::EntityEnter:
			return ParseEntityEnterPacket(raw, parser, clock);
		case PacketBaseType::EntityLeave:
			return ParseEntityLeavePacket(raw, parser, clock);
		case PacketBaseType::PlayerEntity:
			return ParsePlayerEntityPacket(raw, parser, clock);
		case PacketBaseType::Camera:
			return ParseCameraPacket(raw, parser, clock);
		case PacketBaseType::Map:
			return ParseMapPacket(raw, parser, clock);
		case PacketBaseType::CameraFreeLook:
			return ParseCameraFreeLookPacket(raw, parser, clock);
		case PacketBaseType::CameraMode:
			return ParseCameraModePacket(raw, parser, clock);
		case PacketBaseType::CruiseState:
			return ParseCruiseStatePacket(raw, parser, clock);
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
		case 0x1D:
		{
			uint32_t x;
			TakeInto(raw, x);
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

ReplayResult<EntityMethodPacket> rp::ParseEntityMethodPacket(std::span<const Byte>& data, PacketParser& parser, float clock)
{
	EntityMethodPacket packet;
	packet.Type = PacketBaseType::EntityMethod;
	packet.Clock = clock;

	auto err = [data]()
	{
		return PA_REPLAY_ERROR("Failed to parse EntityMethodPacket: {}", FormatBytes(data));
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
		return PA_REPLAY_ERROR("Invalid payload size on EntityMethodPacket: {} != {}", data.size(), size);
	}

	if (!parser.Entities.contains(packet.EntityId))
	{
		return PA_REPLAY_ERROR("Entity {} does not exist for EntityMethodPacket", packet.EntityId);
	}
	const uint16_t entityType = parser.Entities.at(packet.EntityId).Type;

	const int specId = entityType - 1;
	if (specId < 0 || specId >= parser.Specs.size())
	{
		return PA_REPLAY_ERROR("Missing EntitySpec {} for EntityMethodPacket", specId);
	}
	const EntitySpec& spec = parser.Specs[specId];

	if (packet.MethodId >= spec.ClientMethods.size())
	{
		return PA_REPLAY_ERROR("Invalid methodId {} for EntityMethodPacket", packet.MethodId);
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
			return PA_REPLAY_ERROR("Failed to parse value for EntityMethodPacket");
		}
	}

	parser.Callbacks.Invoke(packet);
	return packet;
}

ReplayResult<EntityCreatePacket> rp::ParseEntityCreatePacket(std::span<const Byte>& data, PacketParser& parser, float clock)
{
	EntityCreatePacket packet;
	packet.Type = PacketBaseType::EntityCreate;
	packet.Clock = clock;

	auto err = [data]()
	{
		return PA_REPLAY_ERROR("Failed to parse EntityCreatePacket: {}", FormatBytes(data));
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
		return PA_REPLAY_ERROR("Invalid payload size on EntityCreatePacket: {} != {}", data.size(), size);
	}

	uint8_t propertyCount = 0;
	if (!TakeInto(data, propertyCount))
	{
		return PA_REPLAY_ERROR("Failed to get property count for EntityCreatePacket");
	}

	const int specId = packet.EntityType - 1;
	if (specId < 0 || specId >= parser.Specs.size())
	{
		return PA_REPLAY_ERROR("Missing EntitySpec {} for EntityCreatePacket", specId);
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
			return PA_REPLAY_ERROR("Failed to get propertyId for EntityCreatePacket, not enough data");
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
				return PA_REPLAY_ERROR("Failed to parse value for EntityCreatePacket");
			}
		}
		else
		{
			return PA_REPLAY_ERROR("Failed to get propertyId for EntityCreatePacket, out of bounds");
		}
	}

	parser.Entities.insert_or_assign(packet.EntityId, Entity{ packet.EntityType, spec, {}, clientPropertyValues });

	parser.Callbacks.Invoke(packet);
	return packet;
}

ReplayResult<EntityPropertyPacket> rp::ParseEntityPropertyPacket(std::span<const Byte>& data, PacketParser& parser, float clock)
{
	EntityPropertyPacket packet;
	packet.Clock = clock;
	packet.Type = PacketBaseType::EntityProperty;

	auto err = [data]()
	{
		return PA_REPLAY_ERROR("Failed to parse EntityPropertyPacket: {}", FormatBytes(data));
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
		return PA_REPLAY_ERROR("Invalid payload size on EntityPropertyPacket: {} != {}", data.size(), size);
	}

	if (!parser.Entities.contains(packet.EntityId))
	{
		return PA_REPLAY_ERROR("Entity {} does not exist for EntityPropertyPacket", packet.EntityId);
	}
	const uint16_t entityType = parser.Entities.at(packet.EntityId).Type;

	const int specId = entityType - 1;
	if (specId < 0 || specId >= parser.Specs.size())
	{
		return PA_REPLAY_ERROR("Missing EntitySpec {} for EntityPropertyPacket", specId);
	}
	const EntitySpec& spec = parser.Specs[specId];

	if (packet.MethodId >= spec.ClientProperties.size())
	{
		return PA_REPLAY_ERROR("Invalid methodId {} for EntityPropertyPacket", packet.MethodId);
	}
	const Property& property = spec.ClientProperties[packet.MethodId];
	packet.PropertyName = property.Name;

	if (!parser.Entities.contains(packet.EntityId))
	{
		return PA_REPLAY_ERROR("Entity {} does not exist for EntityPropertyPacket", packet.EntityId);
	}
	Entity& entity = parser.Entities.at(packet.EntityId);

	if (std::optional<ArgValue> value = ParseValue(data, property.Type))
	{
		packet.Value = value.value();
		entity.ClientPropertiesValues.insert_or_assign(property.Name, value.value());
	}
	else
	{
		return PA_REPLAY_ERROR("Failed to parse value for EntityPropertyPacket");
	}

	parser.Callbacks.Invoke(packet);
	return packet;
}

ReplayResult<BasePlayerCreatePacket> rp::ParseBasePlayerCreatePacket(std::span<const Byte>& data, PacketParser& parser, float clock)
{
	BasePlayerCreatePacket packet;
	packet.Clock = clock;
	packet.Type = PacketBaseType::BasePlayerCreate;

	auto err = [data]()
	{
		return PA_REPLAY_ERROR("Failed to parse CellPlayerCreatePacket: {}", FormatBytes(data));
	};

	if (!TakeInto(data, packet.EntityId))
		return err();
	if (!TakeInto(data, packet.EntityType))
		return err();

	const int specId = packet.EntityType - 1;
	if (specId < 0 || specId >= parser.Specs.size())
	{
		return PA_REPLAY_ERROR("Missing EntitySpec {} for BasePlayerCreatePacket", specId);
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
			return PA_REPLAY_ERROR("Failed to parse value for EntityCreatePacket");
		}
	}

	parser.Entities.emplace(packet.EntityId, Entity{ packet.EntityType, spec, basePropertyValues, {} });  // TODO: parse the state

	std::span<const Byte> state = Take(data, data.size());
	packet.Data = { state.begin(), state.end() };

	parser.Callbacks.Invoke(packet);
	return packet;
}

ReplayResult<CellPlayerCreatePacket> rp::ParseCellPlayerCreatePacket(std::span<const Byte>& data, PacketParser& parser, float clock)
{
	CellPlayerCreatePacket packet;
	packet.Type = PacketBaseType::CellPlayerCreate;
	packet.Clock = clock;

	auto err = [data]()
	{
		return PA_REPLAY_ERROR("Failed to parse CellPlayerCreatePacket: {}", FormatBytes(data));
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
		return PA_REPLAY_ERROR("Invalid payload size on CellPlayerCreatePacket: {} != {}", data.size(), size);
	}

	if (!parser.Entities.contains(packet.EntityId))
	{
		return PA_REPLAY_ERROR("Entity {} does not exist for CellPlayerCreatePacket", packet.EntityId);
	}

	const uint16_t entityType = parser.Entities.at(packet.EntityId).Type;

	const int specId = entityType - 1;
	if (specId < 0 || specId >= parser.Specs.size())
	{
		return PA_REPLAY_ERROR("Missing EntitySpec {} for CellPlayerCreatePacket", specId);
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
			return PA_REPLAY_ERROR("Failed to parse value for CellPlayerCreatePacket");
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

ReplayResult<EntityControlPacket> rp::ParseEntityControlPacket(std::span<const Byte>& data, const PacketParser& parser, float clock)
{
	EntityControlPacket packet;
	packet.Type = PacketBaseType::EntityControl;
	packet.Clock = clock;

	auto err = [data]()
	{
		return PA_REPLAY_ERROR("Failed to parse EntityControlPacket: {}", FormatBytes(data));
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

ReplayResult<EntityEnterPacket> rp::ParseEntityEnterPacket(std::span<const Byte>& data, const PacketParser& parser, float clock)
{
	EntityEnterPacket packet;
	packet.Type = PacketBaseType::EntityEnter;
	packet.Clock = clock;

	auto err = [data]()
	{
		return PA_REPLAY_ERROR("Failed to parse EntityEnterPacket: {}", FormatBytes(data));
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

ReplayResult<EntityLeavePacket> rp::ParseEntityLeavePacket(std::span<const Byte>& data, const PacketParser& parser, float clock)
{
	EntityLeavePacket packet;
	packet.Type = PacketBaseType::EntityLeave;
	packet.Clock = clock;

	if (!TakeInto(data, packet.EntityId))
	{
		return PA_REPLAY_ERROR("Failed to parse EntityLeavePacket: {}", FormatBytes(data));
	}

	if (!data.empty())
	{
		LOG_WARN("EntityLeavePacket had {} bytes remaining after parsing", data.size());
	}

	parser.Callbacks.Invoke(packet);
	return packet;
}

ReplayResult<NestedPropertyUpdatePacket> rp::ParseNestedPropertyUpdatePacket(std::span<const Byte>& data, PacketParser& parser, float clock)
{
	NestedPropertyUpdatePacket packet;
	packet.Type = PacketBaseType::NestedPropertyUpdate;
	packet.Clock = clock;

	auto err = [data]()
	{
		return PA_REPLAY_ERROR("Failed to parse NestedPropertyUpdatePacket: {}", FormatBytes(data));
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
		return PA_REPLAY_ERROR("Invalid payload size on NestedPropertyUpdatePacket: {} != {}", data.size(), size);
	}

	std::span<const Byte> payload = Take(data, size);

	if (!parser.Entities.contains(packet.EntityId))
	{
		return PA_REPLAY_ERROR("Entity {} does not exist for EntityPropertyPacket", packet.EntityId);
	}
	packet.EntityPtr = &parser.Entities.at(packet.EntityId);
	const EntitySpec& spec = packet.EntityPtr->Spec;

	BitReader bitReader(payload);
	const int cont = bitReader.Get(1);
	if (cont != 1)
	{
		return PA_REPLAY_ERROR("Invalid first bit {:#04x} in NestedPropertyUpdatePacket payload", cont);
	}

	const int propIndex = bitReader.Get(BitReader::BitsRequired(static_cast<int>(spec.ClientProperties.size())));
	if (propIndex >= spec.ClientProperties.size())
	{
		return PA_REPLAY_ERROR("Property index out of range ({}) for spec in NestedPropertyUpdatePacket", propIndex);
	}

	const Property& prop = spec.ClientProperties[propIndex].get();

	packet.PropertyIndex = propIndex;
	packet.PropertyName = prop.Name;

	if (!packet.EntityPtr->ClientPropertiesValues.contains(prop.Name))
	{
		return PA_REPLAY_ERROR("Entity is missing property value for '{}' in NestedPropertyUpdatePacket", prop.Name);
	}

	PA_TRY(nesting, GetNestedPropertyPath(isSlice, prop.Type, &packet.EntityPtr->ClientPropertiesValues[prop.Name], bitReader));
	packet.Nesting = nesting;

	if (!data.empty())
	{
		LOG_WARN("NestedPropertyUpdatePacket had {} bytes remaining after parsing", data.size());
	}

	parser.Callbacks.Invoke(packet);
	return packet;
}

ReplayResult<PlayerOrientationPacket> rp::ParsePlayerOrientationPacket(std::span<const Byte>& data, const PacketParser& parser, float clock)
{
	PlayerOrientationPacket packet;
	packet.Type = PacketBaseType::PlayerOrientation;
	packet.Clock = clock;

	auto err = [data]()
	{
		return PA_REPLAY_ERROR("Failed to parse PlayerOrientationPacket: {}", FormatBytes(data));
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

ReplayResult<PlayerPositionPacket> rp::ParsePlayerPositionPacketPacket(std::span<const Byte>& data, const PacketParser& parser, float clock)
{
	PlayerPositionPacket packet;
	packet.Type = PacketBaseType::PlayerOrientation;
	packet.Clock = clock;

	auto err = [data]()
	{
		return PA_REPLAY_ERROR("Failed to parse PlayerPositionPacket: {}", FormatBytes(data));
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

ReplayResult<CameraPacket> rp::ParseCameraPacket(std::span<const Byte>& data, const PacketParser& parser, float clock)
{
	CameraPacket packet;
	packet.Type = PacketBaseType::Camera;
	packet.Clock = clock;

	auto err = [data]()
	{
		return PA_REPLAY_ERROR("Failed to parse CameraPacket: {}", FormatBytes(data));
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

ReplayResult<MapPacket> rp::ParseMapPacket(std::span<const Byte>& data, const PacketParser& parser, float clock)
{
	MapPacket packet;
	packet.Type = PacketBaseType::Map;
	packet.Clock = clock;

	auto err = [data]()
	{
		return PA_REPLAY_ERROR("Failed to parse MapPacket: {}", FormatBytes(data));
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

ReplayResult<VersionPacket> rp::ParseVersionPacket(std::span<const Byte>& data, const PacketParser& parser, float clock)
{
	VersionPacket packet;
	packet.Type = PacketBaseType::Version;
	packet.Clock = clock;

	auto err = [data]()
	{
		return PA_REPLAY_ERROR("Failed to parse VersionPacket: {}", FormatBytes(data));
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

ReplayResult<PlayerEntityPacket> rp::ParsePlayerEntityPacket(std::span<const Byte>& data, const PacketParser& parser, float clock)
{
	PlayerEntityPacket packet;
	packet.Type = PacketBaseType::PlayerEntity;
	packet.Clock = clock;

	auto err = [data]()
	{
		return PA_REPLAY_ERROR("Failed to parse PlayerEntityPacket: {}", FormatBytes(data));
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

ReplayResult<CameraModePacket> rp::ParseCameraModePacket(std::span<const Byte>& data, const PacketParser& parser, float clock)
{
	CameraModePacket packet;
	packet.Type = PacketBaseType::CameraMode;
	packet.Clock = clock;

	auto err = [data]()
	{
		return PA_REPLAY_ERROR("Failed to parse CameraModePacket: {}", FormatBytes(data));
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

ReplayResult<CruiseStatePacket> rp::ParseCruiseStatePacket(std::span<const Byte>& data, const PacketParser& parser, float clock)
{
	CruiseStatePacket packet;
	packet.Type = PacketBaseType::CruiseState;
	packet.Clock = clock;

	auto err = [data]()
	{
		return PA_REPLAY_ERROR("Failed to parse CruiseStatePacket: {}", FormatBytes(data));
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

ReplayResult<CameraFreeLookPacket> rp::ParseCameraFreeLookPacket(std::span<const Byte>& data, const PacketParser& parser, float clock)
{
	CameraFreeLookPacket packet;
	packet.Type = PacketBaseType::CameraFreeLook;
	packet.Clock = clock;

	auto err = [data]()
	{
		return PA_REPLAY_ERROR("Failed to parse CameraFreeLookPacket: {}", FormatBytes(data));
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
