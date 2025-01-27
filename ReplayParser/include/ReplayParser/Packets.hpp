// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Core/Bytes.hpp"
#include "Core/Math.hpp"
#include "Core/Preprocessor.hpp"

#include "ReplayParser/Entity.hpp"
#include "ReplayParser/NestedProperty.hpp"
#include "ReplayParser/Types.hpp"

#include <string>
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
};

struct Packet
{
	PacketBaseType Type;
	float Clock;
};

struct UnknownPacket : Packet
{
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
	TypeEntityId EntityId;
	EntityType EntityType;
	std::vector<Byte> Data;
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
	TypeEntityId EntityId;
	TypeSpaceId SpaceId;
	uint16_t Unknown;
	TypeVehicleId VehicleId;
	Vec3 Position;
	Rot3 Rotation;
	std::unordered_map<std::string, ArgValue> Values;
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
	TypeEntityId EntityId;
	bool IsControlled;
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
	TypeEntityId EntityId;
	TypeSpaceId SpaceId;
	TypeVehicleId VehicleId;
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
	TypeEntityId EntityId;
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
	TypeEntityId EntityId;
	EntityType EntityType;
	TypeSpaceId SpaceId;
	TypeVehicleId VehicleId;
	Vec3 Position;
	Rot3 Rotation;
	std::unordered_map<std::string, ArgValue> Values;
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
	TypeEntityId EntityId;
	TypeMethodId MethodId;
	std::string MethodName;
	std::vector<ArgValue> Values;
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
	TypeEntityId EntityId;
	TypeMethodId MethodId;
	std::string PropertyName;
	ArgValue Value;
};

struct PlayerOrientationPacket : Packet
{
	uint32_t Pid;
	uint32_t ParentId;
	Vec3 Position;
	Rot3 Rotation;
};

struct PlayerPositionPacket : Packet
{
	TypeEntityId EntityId;
	TypeVehicleId VehicleId;
	Vec3 Position;
	Vec3 PositionError;
	Rot3 Rotation;
	bool IsError;
};

struct Entity;  // forward declare for PacketParser.hpp

/*
 * https://github.com/Monstrofil/replays_unpack/blob/parser2.0/docs/packets/0x22.md
 */
struct NestedPropertyUpdatePacket : Packet
{
	TypeEntityId EntityId;
	std::string PropertyName;
	size_t PropertyIndex;
	PropertyNesting Nesting;
	Entity* EntityPtr;
};

struct MapPacket : Packet
{
	TypeSpaceId SpaceId;
	int64_t ArenaId;
	uint32_t Unknown1;
	uint32_t Unknown2;
	std::string Name;
	Mat4 Matrix;
	bool Unknown3;
};

struct CameraPacket : Packet
{
	Vec3 Unknown;
	float Unknown2;
	Vec3 AbsolutePosition;
	float Fov;
	Vec3 Position;
	Rot3 Rotation;
	float Unknown3;  // only in newer versions
};

struct VersionPacket : Packet
{
	std::string Version;
};

struct PlayerEntityPacket : Packet
{
	TypeEntityId EntityId;
};

struct CruiseStatePacket : Packet
{
	uint32_t Key;
	int32_t Value;
};

struct CameraFreeLookPacket : Packet
{
	bool Locked;
};

struct CameraModePacket : Packet
{
	uint32_t Mode;
};

struct ResultPacket : Packet
{
	std::string Result;
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
