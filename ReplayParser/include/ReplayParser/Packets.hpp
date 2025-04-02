// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Core/Bytes.hpp"
#include "Core/Math.hpp"
#include "Core/Preprocessor.hpp"
#include "Core/Version.hpp"

#include "ReplayParser/Entity.hpp"
#include "ReplayParser/NestedProperty.hpp"
#include "ReplayParser/Types.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"


using PotatoAlert::Core::Byte;
using PotatoAlert::Core::Mat4;
using PotatoAlert::Core::Rot3;
using PotatoAlert::Core::Vec3;

namespace PotatoAlert::ReplayParser {

typedef int32_t TypeEntityId;
typedef int32_t TypeSpaceId;
typedef int32_t TypeVehicleId;
typedef int32_t TypeMethodId;

struct PacketParseContext
{
	std::vector<EntitySpec> Specs;
	std::unordered_map<TypeEntityId, Entity> Entities;
	Core::Version Version;
};

enum class PacketBaseType
{
	BasePlayerCreate,
	CellPlayerCreate,
	EntityControl,
	EntityEnter,
	EntityLeave,
	EntityCreate,
	EntityProperty,
	EntityMethod,
	PlayerPosition,
	Version,
	PlayerEntity,
	NestedPropertyUpdate,
	// Chat,
	Camera,
	CameraMode,
	Map,
	PlayerOrientation,
	CameraFreeLook,
	CruiseState,
	Result,
	Unknown,
};

struct Packet
{
	float Clock;
};

struct UnknownPacket : Packet
{
	static constexpr PacketBaseType Type = PacketBaseType::Unknown;

	std::vector<Byte> Data;

	PA_API static ReplayResult<UnknownPacket> Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock);
};

/**
 *	This function is called when we get a new player entity.
 *	The data on the stream contains only properties provided by the base.
 *
 *	\param	id			entity id.
 *	\param	type		entity type id.
 *	\param	data		entity data.
 **/
struct BasePlayerCreatePacket : Packet
{
	static constexpr PacketBaseType Type = PacketBaseType::BasePlayerCreate;

	TypeEntityId EntityId;
	TypeEntityType EntityType;
	std::vector<Byte> Data;

	PA_API static ReplayResult<BasePlayerCreatePacket> Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock);
};

/**
 *	This function is called to create the call part of the player entity.
 *	The data on the stream contains only properties provided by the cell.
 *
 *	\param	id			entity id.
 *	\param	spaceID		id of space where to create the entity in.
 *	\param	vehicleID	id of an entity to use as vehicle.
 *	\param	position	position of entity.
 *	\param	yaw			yaw of entity.
 *	\param	pitch		pitch of entity.
 *	\param	roll		roll of entity.
 *	\param	data		entity's data.
 **/
struct CellPlayerCreatePacket : Packet
{
	static constexpr PacketBaseType Type = PacketBaseType::CellPlayerCreate;

	TypeEntityId EntityId;
	TypeSpaceId SpaceId;
	TypeVehicleId VehicleId;
	Vec3 Position;
	Rot3 Rotation;
	std::unordered_map<std::string, ArgValue> Values;

	PA_API static ReplayResult<CellPlayerCreatePacket> Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock);
};

/**
 *	This function is called to tell us that we may now control the given
 *	entity (or not as the case may be).
 *
 *	\param	id			entity id.
 *	\param	control		true if entity is now controlled, false otherwise.
 **/
struct EntityControlPacket : Packet
{
	static constexpr PacketBaseType Type = PacketBaseType::EntityControl;

	TypeEntityId EntityId;
	bool IsControlled;

	PA_API static ReplayResult<EntityControlPacket> Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock);
};

/**
 *	This function is called when the server indicates that an entity has entered
 *	the player's AoI.
 *
 *	It is complicated because it may be called out of order.
 *
 *	\param	id			entity id.
 *	\param	spaceID		id of space where to create the entity in.
 *	\param	vehicleID	id of an entity to use as vehicle.
 **/
struct EntityEnterPacket : Packet
{
	static constexpr PacketBaseType Type = PacketBaseType::EntityEnter;

	TypeEntityId EntityId;
	TypeSpaceId SpaceId;
	TypeVehicleId VehicleId;

	PA_API static ReplayResult<EntityEnterPacket> Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock);
};

/**
 *	This function is called when the server indicates that an entity has left
 *	the player's AoI. It is complicated because it may be called out of order.
 *
 *	\param	id				entity id.
 *	\param	cacheStamps		Unused parameter.
 **/
struct EntityLeavePacket : Packet
{
	static constexpr PacketBaseType Type = PacketBaseType::EntityLeave;

	TypeEntityId EntityId;

	PA_API static ReplayResult<EntityLeavePacket> Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock);
};

/**
 *	This function is called when the server provides us with the details
 *	necessary to create an entity here. The minimum data it could send
 *	is the type of the entity, but for the moment it sends everything.
 *
 *	\param	id			entity id.
 *	\param	type		entity type id.
 *	\param	spaceID		id of space where to create the entity in.
 *	\param	vehicleID	id of an entity to use as vehicle.
 *	\param	position	position of entity.
 *	\param	yaw			yaw of entity.
 *	\param	pitch		pitch of entity.
 *	\param	roll		roll of entity.
 *	\param	data		entity's data.
 *
 *	type - index of python class stored in entities.xml
 *	state - really strange thing... I've tried to understood how data is stored, but have no luck. I guess it is a strange form of PyDict.
 **/
struct EntityCreatePacket : Packet
{
	static constexpr PacketBaseType Type = PacketBaseType::EntityCreate;

	TypeEntityId EntityId;
	TypeEntityType EntityType;
	TypeSpaceId SpaceId;
	TypeVehicleId VehicleId;
	Vec3 Position;
	Rot3 Rotation;
	std::unordered_map<std::string, ArgValue> Values;

	PA_API static ReplayResult<EntityCreatePacket> Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock);
};

/**
 *	This method is called when we receive a script method call message
 *	for one of our client-side entities from the server.
 *
 *	\param	id				entity id.
 *	\param	messageID		message id.
 *	\param	data			message data.
 **/
struct EntityMethodPacket : Packet
{
	static constexpr PacketBaseType Type = PacketBaseType::EntityMethod;

	TypeEntityId EntityId;
	TypeMethodId MethodId;
	std::string MethodName;
	std::vector<ArgValue> Values;

	PA_API static ReplayResult<EntityMethodPacket> Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock);
};

/**
 *	This method is called when we receive a property update message
 *	for one of our client-side entities from the server, aka server wants us to change a property on a client.
 *
 *	\param	id				entity id.
 *	\param	messageID		message id.
 *	\param	data			message data.
 **/
struct EntityPropertyPacket : Packet
{
	static constexpr PacketBaseType Type = PacketBaseType::EntityProperty;

	TypeEntityId EntityId;
	TypeMethodId PropertyId;
	std::string PropertyName;
	ArgValue Value;

	PA_API static ReplayResult<EntityPropertyPacket> Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock);
};

struct PlayerOrientationPacket : Packet
{
	static constexpr PacketBaseType Type = PacketBaseType::PlayerOrientation;

	TypeEntityId EntityId;
	TypeVehicleId VehicleId;
	Vec3 Position;
	Rot3 Rotation;

	PA_API static ReplayResult<PlayerOrientationPacket> Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock);
};

struct PlayerPositionPacket : Packet
{
	static constexpr PacketBaseType Type = PacketBaseType::PlayerPosition;

	TypeEntityId EntityId;
	TypeVehicleId VehicleId;
	Vec3 Position;
	Vec3 PositionError;
	Rot3 Rotation;
	bool IsError;

	PA_API static ReplayResult<PlayerPositionPacket> Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock);
};

/*
 * https://github.com/Monstrofil/replays_unpack/blob/parser2.0/docs/packets/0x22.md
 */
struct NestedPropertyUpdatePacket : Packet
{
	static constexpr PacketBaseType Type = PacketBaseType::NestedPropertyUpdate;

	TypeEntityId EntityId;
	std::string PropertyName;
	size_t PropertyIndex;
	PropertyNesting Nesting;
	Entity* EntityPtr;

	PA_API static ReplayResult<NestedPropertyUpdatePacket> Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock);
};

struct MapPacket : Packet
{
	static constexpr PacketBaseType Type = PacketBaseType::Map;

	TypeSpaceId SpaceId;
	int64_t ArenaId;
	uint32_t Unknown1;
	uint32_t Unknown2;
	std::string Name;
	Mat4 Matrix;
	bool Unknown3;

	PA_API static ReplayResult<MapPacket> Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock);
};

struct CameraPacket : Packet
{
	static constexpr PacketBaseType Type = PacketBaseType::Camera;

	Vec3 Unknown;
	float Unknown2;
	Vec3 AbsolutePosition;
	float Fov;
	Vec3 Position;
	Rot3 Rotation;
	float Unknown3;  // only in newer versions

	PA_API static ReplayResult<CameraPacket> Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock);
};

struct VersionPacket : Packet
{
	static constexpr PacketBaseType Type = PacketBaseType::Version;

	std::string Version;

	PA_API static ReplayResult<VersionPacket> Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock);
};

struct PlayerEntityPacket : Packet
{
	static constexpr PacketBaseType Type = PacketBaseType::PlayerEntity;

	TypeEntityId EntityId;

	PA_API static ReplayResult<PlayerEntityPacket> Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock);
};

struct CruiseStatePacket : Packet
{
	static constexpr PacketBaseType Type = PacketBaseType::CruiseState;

	uint32_t Key;
	int32_t Value;

	PA_API static ReplayResult<CruiseStatePacket> Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock);
};

struct CameraFreeLookPacket : Packet
{
	static constexpr PacketBaseType Type = PacketBaseType::CameraFreeLook;

	bool Locked;

	PA_API static ReplayResult<CameraFreeLookPacket> Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock);
};

struct CameraModePacket : Packet
{
	static constexpr PacketBaseType Type = PacketBaseType::CameraMode;

	uint32_t Mode;

	PA_API static ReplayResult<CameraModePacket> Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock);
};

struct ResultPacket : Packet
{
	static constexpr PacketBaseType Type = PacketBaseType::Result;

	std::string Result;

	PA_API static ReplayResult<ResultPacket> Parse(std::span<const Byte>& data, PacketParseContext& ctx, float clock);
};

#define PA_RP_PACKETS(X)          \
	X(BasePlayerCreatePacket)     \
	X(CellPlayerCreatePacket)     \
	X(EntityControlPacket)        \
	X(EntityEnterPacket)          \
	X(EntityLeavePacket)          \
	X(EntityCreatePacket)         \
	X(EntityMethodPacket)         \
	X(EntityPropertyPacket)       \
	X(PlayerPositionPacket)       \
	X(PlayerOrientationPacket)    \
	X(MapPacket)                  \
	X(NestedPropertyUpdatePacket) \
	X(VersionPacket)              \
	X(CameraPacket)               \
	X(PlayerEntityPacket)         \
	X(UnknownPacket)              \
	X(CruiseStatePacket)          \
	X(CameraFreeLookPacket)       \
	X(CameraModePacket)           \
	X(ResultPacket)

typedef std::variant<
		PA_CHAIN_COMMA(PA_RP_PACKETS(PA_NOARG))
> PacketType;

}  // namespace PotatoAlert::ReplayParser

#pragma clang diagnostic pop
