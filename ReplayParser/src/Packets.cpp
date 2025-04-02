// Copyright 2025 <github.com/razaqq>

#include "Core/Bytes.hpp"
#include "Core/Log.hpp"
#include "Core/Version.hpp"

#include "ReplayParser/Entity.hpp"
#include "ReplayParser/Packets.hpp"
#include "ReplayParser/Result.hpp"

#include <cstdint>
#include <span>
#include <string>


using PotatoAlert::Core::Take;
using PotatoAlert::Core::TakeInto;
using PotatoAlert::Core::TakeString;
using PotatoAlert::Core::FormatBytes;
using PotatoAlert::Core::Version;
using PotatoAlert::ReplayParser::Entity;
using PotatoAlert::ReplayParser::TypeEntityType;
using PotatoAlert::ReplayParser::BasePlayerCreatePacket;
using PotatoAlert::ReplayParser::CameraFreeLookPacket;
using PotatoAlert::ReplayParser::CameraModePacket;
using PotatoAlert::ReplayParser::CameraPacket;
using PotatoAlert::ReplayParser::CellPlayerCreatePacket;
using PotatoAlert::ReplayParser::CruiseStatePacket;
using PotatoAlert::ReplayParser::EntityCreatePacket;
using PotatoAlert::ReplayParser::EntityControlPacket;
using PotatoAlert::ReplayParser::EntityEnterPacket;
using PotatoAlert::ReplayParser::EntityLeavePacket;
using PotatoAlert::ReplayParser::EntityMethodPacket;
using PotatoAlert::ReplayParser::EntityPropertyPacket;
using PotatoAlert::ReplayParser::PlayerOrientationPacket;
using PotatoAlert::ReplayParser::PlayerPositionPacket;
using PotatoAlert::ReplayParser::NestedPropertyUpdatePacket;
using PotatoAlert::ReplayParser::MapPacket;
using PotatoAlert::ReplayParser::VersionPacket;
using PotatoAlert::ReplayParser::PlayerEntityPacket;
using PotatoAlert::ReplayParser::ResultPacket;
using PotatoAlert::ReplayParser::UnknownPacket;
using PotatoAlert::ReplayParser::ReplayResult;

ReplayResult<UnknownPacket> UnknownPacket::Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock)
{
	UnknownPacket packet;
	packet.Clock = clock;
	std::span<const Byte> d = Take(data, data.size());
	packet.Data = { d.begin(),d.end() };
	return packet;
}

ReplayResult<BasePlayerCreatePacket> BasePlayerCreatePacket::Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock)
{
	BasePlayerCreatePacket packet;
	packet.Clock = clock;

	auto err = [data]()
	{
		return PA_REPLAY_ERROR("Failed to parse CellPlayerCreatePacket: {}", FormatBytes(data));
	};

	if (!TakeInto(data, packet.EntityId))
		return err();
	if (!TakeInto(data, packet.EntityType))
		return err();

	const int specId = static_cast<std::underlying_type_t<TypeEntityType>>(packet.EntityType) - 1;
	if (specId < 0 || static_cast<size_t>(specId) >= ctx.Specs.size())
	{
		return PA_REPLAY_ERROR("Missing EntitySpec {} for BasePlayerCreatePacket", specId);
	}
	const EntitySpec& spec = ctx.Specs[specId];

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

	ctx.Entities.emplace(packet.EntityId, Entity{ packet.EntityType, spec, basePropertyValues, {}, {} });  // TODO: parse the state

	std::span<const Byte> state = Take(data, data.size());
	packet.Data = { state.begin(), state.end() };

	return packet;
}

ReplayResult<CellPlayerCreatePacket> CellPlayerCreatePacket::Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock)
{
	CellPlayerCreatePacket packet;
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

	if (!ctx.Entities.contains(packet.EntityId))
	{
		return PA_REPLAY_ERROR("Entity {} does not exist for CellPlayerCreatePacket", packet.EntityId);
	}

	const TypeEntityType entityType = ctx.Entities.at(packet.EntityId).Type;

	const int specId = static_cast<std::underlying_type_t<TypeEntityType>>(entityType) - 1;
	if (specId < 0 || static_cast<size_t>(specId) >= ctx.Specs.size())
	{
		return PA_REPLAY_ERROR("Missing EntitySpec {} for CellPlayerCreatePacket", specId);
	}
	const EntitySpec& spec = ctx.Specs[specId];

	if (!ctx.Entities.contains(packet.EntityId))
	{
		LOG_WARN("CellPlayerCreatePacket created non-existing entity {}", packet.EntityId);
		ctx.Entities.emplace(packet.EntityId, Entity{ entityType, spec, {}, {}, {} });
	}

	packet.Values.reserve(spec.ClientPropertiesInternal.size());
	for (auto property : spec.ClientPropertiesInternal)
	{
		PA_TRY_OR_ELSE(value, ParseValue(data, property.get().Type),
		{
			return PA_REPLAY_ERROR("Failed to parse value for CellPlayerCreatePacket: {}", error);
		});

		packet.Values[property.get().Name] = value;
		ctx.Entities.at(packet.EntityId).ClientPropertiesValues.emplace(property.get().Name, value);
	}

	uint8_t unknown;
	TakeInto(data, unknown);

	if (!data.empty())
	{
		LOG_WARN("CellPlayerCreatePacket had {} bytes remaining after parsing", data.size());
	}

	return packet;
}

ReplayResult<EntityControlPacket> EntityControlPacket::Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock)
{
	EntityControlPacket packet;
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

	return packet;
}

ReplayResult<EntityEnterPacket> EntityEnterPacket::Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock)
{
	EntityEnterPacket packet;
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

	return packet;
}

ReplayResult<EntityLeavePacket> EntityLeavePacket::Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock)
{
	EntityLeavePacket packet;
	packet.Clock = clock;

	if (!TakeInto(data, packet.EntityId))
	{
		return PA_REPLAY_ERROR("Failed to parse EntityLeavePacket: {}", FormatBytes(data));
	}

	if (!data.empty())
	{
		LOG_WARN("EntityLeavePacket had {} bytes remaining after parsing", data.size());
	}

	return packet;
}

ReplayResult<EntityCreatePacket> EntityCreatePacket::Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock)
{
	EntityCreatePacket packet;
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

	if (ctx.Entities.contains(packet.EntityId))
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

	const int specId = static_cast<std::underlying_type_t<TypeEntityType>>(packet.EntityType) - 1;
	if (specId < 0 || static_cast<size_t>(specId) >= ctx.Specs.size())
	{
		return PA_REPLAY_ERROR("Missing EntitySpec {} for EntityCreatePacket", specId);
	}
	const EntitySpec& spec = ctx.Specs[specId];

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

	ctx.Entities.insert_or_assign(packet.EntityId, Entity{ packet.EntityType, spec, {}, clientPropertyValues, {} });

	return packet;
}

ReplayResult<EntityMethodPacket> EntityMethodPacket::Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock)
{
	EntityMethodPacket packet;
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

	if (!ctx.Entities.contains(packet.EntityId))
	{
		return PA_REPLAY_ERROR("Entity {} does not exist for EntityMethodPacket", packet.EntityId);
	}
	const uint16_t entityType = static_cast<std::underlying_type_t<TypeEntityType>>(ctx.Entities.at(packet.EntityId).Type);

	const int specId = entityType - 1;
	if (specId < 0 || static_cast<size_t>(specId) >= ctx.Specs.size())
	{
		return PA_REPLAY_ERROR("Missing EntitySpec {} for EntityMethodPacket", specId);
	}
	const EntitySpec& spec = ctx.Specs[specId];

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

	return packet;
}

ReplayResult<EntityPropertyPacket> EntityPropertyPacket::Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock)
{
	EntityPropertyPacket packet;
	packet.Clock = clock;

	auto err = [data]()
	{
		return PA_REPLAY_ERROR("Failed to parse EntityPropertyPacket: {}", FormatBytes(data));
	};

	if (!TakeInto(data, packet.EntityId))
		return err();
	if (!TakeInto(data, packet.PropertyId))
		return err();

	uint32_t size;
	if (!TakeInto(data, size))
		return err();

	if (data.size() != size)
	{
		return PA_REPLAY_ERROR("Invalid payload size on EntityPropertyPacket: {} != {}", data.size(), size);
	}

	if (!ctx.Entities.contains(packet.EntityId))
	{
		return PA_REPLAY_ERROR("Entity {} does not exist for EntityPropertyPacket", packet.EntityId);
	}
	const uint16_t entityType = static_cast<std::underlying_type_t<TypeEntityType>>(ctx.Entities.at(packet.EntityId).Type);

	const int specId = entityType - 1;
	if (specId < 0 || static_cast<size_t>(specId) >= ctx.Specs.size())
	{
		return PA_REPLAY_ERROR("Missing EntitySpec {} for EntityPropertyPacket", specId);
	}
	const EntitySpec& spec = ctx.Specs[specId];

	if (static_cast<size_t>(packet.PropertyId) >= spec.ClientProperties.size())
	{
		return PA_REPLAY_ERROR("Invalid methodId {} for EntityPropertyPacket", packet.PropertyId);
	}
	const Property& property = spec.ClientProperties[packet.PropertyId];
	packet.PropertyName = property.Name;

	if (!ctx.Entities.contains(packet.EntityId))
	{
		return PA_REPLAY_ERROR("Entity {} does not exist for EntityPropertyPacket", packet.EntityId);
	}
	Entity& entity = ctx.Entities.at(packet.EntityId);

	PA_TRY_OR_ELSE(value, ParseValue(data, property.Type),
	{
		return PA_REPLAY_ERROR("Failed to parse value for EntityPropertyPacket: {}", error);
	});

	packet.Value = value;
	entity.ClientPropertiesValues.insert_or_assign(property.Name, value);

	return packet;
}

ReplayResult<PlayerOrientationPacket> PlayerOrientationPacket::Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock)
{
	PlayerOrientationPacket packet;
	packet.Clock = clock;

	auto err = [data]()
	{
		return PA_REPLAY_ERROR("Failed to parse PlayerOrientationPacket: {}", FormatBytes(data));
	};

	if (!TakeInto(data, packet.EntityId))
		return err();
	if (!TakeInto(data, packet.VehicleId))
		return err();
	if (!TakeInto(data, packet.Position))
		return err();
	if (!TakeInto(data, packet.Rotation))
		return err();

	if (!data.empty())
	{
		LOG_WARN("PlayerOrientationPacket had {} bytes remaining after parsing", data.size());
	}

	return packet;
}

ReplayResult<PlayerPositionPacket> PlayerPositionPacket::Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock)
{
	PlayerPositionPacket packet;
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

	return packet;
}

ReplayResult<NestedPropertyUpdatePacket> NestedPropertyUpdatePacket::Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock)
{
	NestedPropertyUpdatePacket packet;
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

	if (!ctx.Entities.contains(packet.EntityId))
	{
		return PA_REPLAY_ERROR("Entity {} does not exist for EntityPropertyPacket", packet.EntityId);
	}
	packet.EntityPtr = &ctx.Entities.at(packet.EntityId);
	const EntitySpec& spec = packet.EntityPtr->Spec;

	BitReader bitReader(payload);
	const int cont = bitReader.Get(1);
	if (cont != 1)
	{
		return PA_REPLAY_ERROR("Invalid first bit {:#04x} in NestedPropertyUpdatePacket payload", cont);
	}

	const size_t propIndex = static_cast<size_t>(bitReader.Get((size_t)BitReader::BitsRequired(spec.ClientProperties.size())));
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

	return packet;
}

ReplayResult<MapPacket> MapPacket::Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock)
{
	MapPacket packet;
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

	return packet;
}

ReplayResult<CameraPacket> CameraPacket::Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock)
{
	CameraPacket packet;
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

	return packet;
}

ReplayResult<VersionPacket> VersionPacket::Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock)
{
	VersionPacket packet;
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

	return packet;
}

ReplayResult<PlayerEntityPacket> PlayerEntityPacket::Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock)
{
	PlayerEntityPacket packet;
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

	return packet;
}

ReplayResult<CruiseStatePacket> CruiseStatePacket::Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock)
{
	CruiseStatePacket packet;
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

	return packet;
}

ReplayResult<CameraFreeLookPacket> CameraFreeLookPacket::Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock)
{
	CameraFreeLookPacket packet;
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

	return packet;
}

ReplayResult<CameraModePacket> CameraModePacket::Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock)
{
	CameraModePacket packet;
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

	return packet;
}

ReplayResult<ResultPacket> ResultPacket::Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock)
{
	ResultPacket packet;
	packet.Clock = clock;

	uint32_t size;
	if (!TakeInto(data, size))
		return PA_REPLAY_ERROR("Failed to parse ResultPacket: {}", FormatBytes(data));
	if (!TakeString(data, packet.Result, size))
		return PA_REPLAY_ERROR("Failed to parse ResultPacket: {}", FormatBytes(data));
	if (size != packet.Result.size())
		return PA_REPLAY_ERROR("Invalid length of ResultPacket: {} != {}", size, packet.Result.size());

	return packet;
}
