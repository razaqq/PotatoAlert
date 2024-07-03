// Copyright 2021 <github.com/razaqq>

#include "Core/Bytes.hpp"
#include "Core/Log.hpp"
#include "Core/Version.hpp"

#include "ReplayParser/BitReader.hpp"
#include "ReplayParser/PacketParser.hpp"
#include "ReplayParser/Packets.hpp"
#include "ReplayParser/Result.hpp"

#include <cstdint>
#include <optional>
#include <span>
#include <unordered_map>
#include <variant>
#include <vector>


using namespace PotatoAlert::ReplayParser;
using PotatoAlert::Core::FormatBytes;
using PotatoAlert::Core::Take;
using PotatoAlert::Core::TakeInto;
using PotatoAlert::Core::TakeString;
using PotatoAlert::Core::Version;

namespace {

static constexpr bool IsPacket(PacketBaseType type, uint32_t id, Version version)
{
	switch (type)
	{
		case PacketBaseType::BasePlayerCreate:     return id == 0x00;
		case PacketBaseType::CellPlayerCreate:     return id == 0x01;
		case PacketBaseType::EntityControl:        return id == 0x02;
		case PacketBaseType::EntityEnter:          return id == 0x03;
		case PacketBaseType::EntityLeave:          return id == 0x04;
		case PacketBaseType::EntityCreate:         return id == 0x05;
		case PacketBaseType::EntityProperty:       return id == 0x07;
		case PacketBaseType::EntityMethod:         return id == 0x08;
		case PacketBaseType::PlayerPosition:       return id == 0x0A;
		case PacketBaseType::Version:              return id == 0x16;
		case PacketBaseType::PlayerEntity:         return id == 0x20;
		case PacketBaseType::NestedPropertyUpdate:
		{
			if (version <= Version(12, 5, 0))
				return id == 0x22;
			return id == 0x23;
		}
		// case PacketBaseType::Chat: return 0x23;
		case PacketBaseType::Camera:               return id == 0x24;
		case PacketBaseType::CameraMode:           return id == 0x26;
		case PacketBaseType::Map:
		{
			if (version <= Version(12, 5, 0))
				return id == 0x27;
			return id == 0x28;
		}
		case PacketBaseType::PlayerOrientation:    return id == 0x2B;
		case PacketBaseType::CameraFreeLook:       return id == 0x2E;
		case PacketBaseType::CruiseState:          return id == 0x31;
		case PacketBaseType::Result:
		{
			if (version >= Version(12, 6, 0))
				return id == 0x22;
			return false;
		}
	}

	return false;
}

static ReplayResult<EntityMethodPacket> ParseEntityMethodPacket(std::span<const Byte>& data, PacketParser& parser, float clock)
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
	if (specId < 0 || static_cast<size_t>(specId) >= parser.Specs.size())
	{
		return PA_REPLAY_ERROR("Missing EntitySpec {} for EntityMethodPacket", specId);
	}
	const EntitySpec& spec = parser.Specs[specId];

	if (static_cast<size_t>(packet.MethodId) >= spec.ClientMethods.size())
	{
		return PA_REPLAY_ERROR("Invalid methodId {} for EntityMethodPacket", packet.MethodId);
	}
	const Method& method = spec.ClientMethods[packet.MethodId];
	packet.MethodName = method.Name;

	packet.Values.reserve(method.Args.size());
	for (const ArgType& argType : method.Args)
	{
		PA_TRY_OR_ELSE(value, ParseValue(data, argType),
		{
			return PA_REPLAY_ERROR("Failed to parse value for EntityMethodPacket: {}", error);
		});
		packet.Values.emplace_back(value);
	}

	parser.Callbacks.Invoke(packet);
	return packet;
}

static ReplayResult<EntityCreatePacket> ParseEntityCreatePacket(std::span<const Byte>& data, PacketParser& parser, float clock)
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
	if (specId < 0 || static_cast<size_t>(specId) >= parser.Specs.size())
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

			PA_TRY_OR_ELSE(value, ParseValue(data, type),
			{
				return PA_REPLAY_ERROR("Failed to parse value for EntityCreatePacket: {}", error);
			});

			packet.Values[name] = value;
			clientPropertyValues.insert_or_assign(name, value);
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

static ReplayResult<EntityPropertyPacket> ParseEntityPropertyPacket(std::span<const Byte>& data, PacketParser& parser, float clock)
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
	if (specId < 0 || static_cast<size_t>(specId) >= parser.Specs.size())
	{
		return PA_REPLAY_ERROR("Missing EntitySpec {} for EntityPropertyPacket", specId);
	}
	const EntitySpec& spec = parser.Specs[specId];

	if (static_cast<size_t>(packet.MethodId) >= spec.ClientProperties.size())
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

	PA_TRY_OR_ELSE(value, ParseValue(data, property.Type),
	{
		return PA_REPLAY_ERROR("Failed to parse value for EntityPropertyPacket: {}", error);
	});

	packet.Value = value;
	entity.ClientPropertiesValues.insert_or_assign(property.Name, value);

	parser.Callbacks.Invoke(packet);
	return packet;
}

static ReplayResult<BasePlayerCreatePacket> ParseBasePlayerCreatePacket(std::span<const Byte>& data, PacketParser& parser, float clock)
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
	if (specId < 0 || static_cast<size_t>(specId) >= parser.Specs.size())
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

		PA_TRY_OR_ELSE(value, ParseValue(data, type),
		{
			return PA_REPLAY_ERROR("Failed to parse value for EntityCreatePacket: {}", error);
		});

		basePropertyValues.emplace(name, value);
	}

	parser.Entities.emplace(packet.EntityId, Entity{ packet.EntityType, spec, basePropertyValues, {} });  // TODO: parse the state

	std::span<const Byte> state = Take(data, data.size());
	packet.Data = { state.begin(), state.end() };

	parser.Callbacks.Invoke(packet);
	return packet;
}

static ReplayResult<CellPlayerCreatePacket> ParseCellPlayerCreatePacket(std::span<const Byte>& data, PacketParser& parser, float clock)
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
	if (specId < 0 || static_cast<size_t>(specId) >= parser.Specs.size())
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
		PA_TRY_OR_ELSE(value, ParseValue(data, property.get().Type),
		{
			return PA_REPLAY_ERROR("Failed to parse value for CellPlayerCreatePacket: {}", error);
		});

		packet.Values[property.get().Name] = value;
		parser.Entities.at(packet.EntityId).ClientPropertiesValues.emplace(property.get().Name, value);
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

static ReplayResult<EntityControlPacket> ParseEntityControlPacket(std::span<const Byte>& data, const PacketParser& parser, float clock)
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

static ReplayResult<EntityEnterPacket> ParseEntityEnterPacket(std::span<const Byte>& data, const PacketParser& parser, float clock)
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

static ReplayResult<EntityLeavePacket> ParseEntityLeavePacket(std::span<const Byte>& data, const PacketParser& parser, float clock)
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

static ReplayResult<NestedPropertyUpdatePacket> ParseNestedPropertyUpdatePacket(std::span<const Byte>& data, PacketParser& parser, float clock)
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
	if (static_cast<size_t>(propIndex) >= spec.ClientProperties.size())
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

static ReplayResult<PlayerOrientationPacket> ParsePlayerOrientationPacket(std::span<const Byte>& data, const PacketParser& parser, float clock)
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

static ReplayResult<PlayerPositionPacket> ParsePlayerPositionPacketPacket(std::span<const Byte>& data, const PacketParser& parser, float clock)
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

static ReplayResult<CameraPacket> ParseCameraPacket(std::span<const Byte>& data, const PacketParser& parser, float clock)
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

static ReplayResult<MapPacket> ParseMapPacket(std::span<const Byte>& data, const PacketParser& parser, float clock)
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

static ReplayResult<VersionPacket> ParseVersionPacket(std::span<const Byte>& data, const PacketParser& parser, float clock)
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

static ReplayResult<PlayerEntityPacket> ParsePlayerEntityPacket(std::span<const Byte>& data, const PacketParser& parser, float clock)
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

static ReplayResult<CameraModePacket> ParseCameraModePacket(std::span<const Byte>& data, const PacketParser& parser, float clock, Version version)
{
	CameraModePacket packet;
	packet.Type = PacketBaseType::CameraMode;
	packet.Clock = clock;

	auto err = [data]()
	{
		return PA_REPLAY_ERROR("Failed to parse CameraModePacket: {}", FormatBytes(data));
	};

	if (version >= Version(12, 6))
	{
		uint32_t unknown;
		if (!TakeInto(data, unknown))
			return err();
	}

	if (!TakeInto(data, packet.Mode))
		return err();

	if (version >= Version(12, 6))
	{
		bool unknown2;
		if (!TakeInto(data, unknown2))
			return err();
	}

	if (version >= Version(12, 6))
	{
		bool unknown3;
		if (!TakeInto(data, unknown3))
			return err();
	}

	if (!data.empty())
	{
		LOG_WARN("CameraModePacket had {} bytes remaining after parsing", data.size());
	}

	parser.Callbacks.Invoke(packet);
	return packet;
}

static ReplayResult<CruiseStatePacket> ParseCruiseStatePacket(std::span<const Byte>& data, const PacketParser& parser, float clock)
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

static ReplayResult<CameraFreeLookPacket> ParseCameraFreeLookPacket(std::span<const Byte>& data, const PacketParser& parser, float clock)
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

static ReplayResult<ResultPacket> ParseResultPacket(std::span<const Byte>&data, const PacketParser &parser, float clock)
{
	ResultPacket packet;
	packet.Type = PacketBaseType::Result;
	packet.Clock = clock;

	uint32_t size;
	if (!TakeInto(data, size))
		return PA_REPLAY_ERROR("Failed to parse ResultPacket: {}", FormatBytes(data));
	if (!TakeString(data, packet.Result, size))
		return PA_REPLAY_ERROR("Failed to parse ResultPacket: {}", FormatBytes(data));
	if (size != packet.Result.size())
		return PA_REPLAY_ERROR("Invalid length of ResultPacket: {} != {}", size, packet.Result.size());

	parser.Callbacks.Invoke(packet);
	return packet;
}


}  // namespace

ReplayResult<PacketType> PotatoAlert::ReplayParser::ParsePacket(std::span<const Byte>& data, PacketParser& parser, Version version)
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

	std::span<const Byte> raw = Take(data, size);

	if (IsPacket(PacketBaseType::EntityCreate, type, version))
		return ParseEntityCreatePacket(raw, parser, clock);
	if (IsPacket(PacketBaseType::BasePlayerCreate, type, version))
		return ParseBasePlayerCreatePacket(raw, parser, clock);
	if (IsPacket(PacketBaseType::CellPlayerCreate, type, version))
		return ParseCellPlayerCreatePacket(raw, parser, clock);
	if (IsPacket(PacketBaseType::EntityMethod, type, version))
		return ParseEntityMethodPacket(raw, parser, clock);
	if (IsPacket(PacketBaseType::EntityProperty, type, version))
		return ParseEntityPropertyPacket(raw, parser, clock);
	if (IsPacket(PacketBaseType::NestedPropertyUpdate, type, version))
		return ParseNestedPropertyUpdatePacket(raw, parser, clock);
	if (IsPacket(PacketBaseType::PlayerPosition, type, version))
		return ParsePlayerPositionPacketPacket(raw, parser, clock);
	if (IsPacket(PacketBaseType::PlayerOrientation, type, version))
		return ParsePlayerOrientationPacket(raw, parser, clock);
	if (IsPacket(PacketBaseType::EntityLeave, type, version))
		return ParseEntityLeavePacket(raw, parser, clock);

#ifndef NDEBUG
	if (IsPacket(PacketBaseType::Version, type, version))
		return ParseVersionPacket(raw, parser, clock);
	if (IsPacket(PacketBaseType::EntityControl, type, version))
		return ParseEntityControlPacket(raw, parser, clock);
	if (IsPacket(PacketBaseType::EntityEnter, type, version))
		return ParseEntityEnterPacket(raw, parser, clock);
	if (IsPacket(PacketBaseType::PlayerEntity, type, version))
		return ParsePlayerEntityPacket(raw, parser, clock);
	if (IsPacket(PacketBaseType::Camera, type, version))
		return ParseCameraPacket(raw, parser, clock);
	if (IsPacket(PacketBaseType::Map, type, version))
		return ParseMapPacket(raw, parser, clock);
	if (IsPacket(PacketBaseType::CameraFreeLook, type, version))
		return ParseCameraFreeLookPacket(raw, parser, clock);
	if (IsPacket(PacketBaseType::CameraMode, type, version))
		return ParseCameraModePacket(raw, parser, clock, version);
	if (IsPacket(PacketBaseType::CruiseState, type, version))
		return ParseCruiseStatePacket(raw, parser, clock);
	if (IsPacket(PacketBaseType::Result, type, version))
		return ParseResultPacket(raw, parser, clock);
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

	return UnknownPacket{};
}
