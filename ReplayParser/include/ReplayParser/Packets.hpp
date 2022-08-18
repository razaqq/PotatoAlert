// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Core/Bytes.hpp"
#include "Core/Math.hpp"
#include "GameFiles.hpp"
#include "Types.hpp"

#include <span>
#include <string>
#include <variant>
#include <vector>


using PotatoAlert::Core::Byte;
using PotatoAlert::Core::Mat4;
using PotatoAlert::Core::Rot3;
using PotatoAlert::Core::Vec3;

namespace PotatoAlert::ReplayParser {

typedef int32_t TypeEntityId;
typedef int32_t TypeSpaceId;
typedef int32_t TypeVehicleId;
typedef int32_t TypeMethodId;
typedef uint16_t TypeEntityType;
#endif

enum class PacketBaseType : uint32_t
{
	BasePlayerCreate     = 0x00,
	CellPlayerCreate     = 0x01,
	EntityControl        = 0x02,
	EntityEnter          = 0x03,
	EntityLeave          = 0x04,
	EntityCreate         = 0x05,
	EntityProperty       = 0x07,
	EntityMethod         = 0x08,
	PlayerPosition       = 0x0A,
	Version              = 0x16,
	PlayerEntity         = 0x20,
	NestedPropertyUpdate = 0x22,
	// Chat              = 0x23,
	Camera               = 0x24,
	CameraMode           = 0x26,
	Map                  = 0x27,
	PlayerOrientation    = 0x2B,
	CameraFreeLook       = 0x2E,
	CruiseState          = 0x31,
};

struct Packet
{
	PacketBaseType Type;
	float Clock;
};

struct UnknownPacket : Packet
{
};

struct InvalidPacket : Packet
{
	std::string Message;
	std::vector<Byte> Raw;
};

/**
 *	This function is called when we get a new player entity.
 *	The data on the stream contains only properties provided by the base.
 *
 *	@param	id			entity id.
 *	@param	type		entity type id.
 *	@param	data		entity data.
 **/
struct BasePlayerCreatePacket : Packet
{
	TypeEntityId EntityId;
	TypeEntityType EntityType;
	std::vector<Byte> Data;
};

/**
 *	This function is called to create the call part of the player entity.
 *	The data on the stream contains only properties provided by the cell.
 *
 *	@param	id			entity id.
 *	@param	spaceID		id of space where to create the entity in.
 *	@param	vehicleID	id of an entity to use as vehicle.
 *	@param	position	position of entity.
 *	@param	yaw			yaw of entity.
 *	@param	pitch		pitch of entity.
 *	@param	roll		roll of entity.
 *	@param	data		entity's data.
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
 *	@param	id			entity id.
 *	@param	control		true if entity is now controlled, false otherwise.
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
 *	@param	id			entity id.
 *	@param	spaceID		id of space where to create the entity in.
 *	@param	vehicleID	id of an entity to use as vehicle.
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
 *	@param	id				entity id.
 *	@param	cacheStamps		Unused parameter.
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
 *	@param	id			entity id.
 *	@param	type		entity type id.
 *	@param	spaceID		id of space where to create the entity in.
 *	@param	vehicleID	id of an entity to use as vehicle.
 *	@param	position	position of entity.
 *	@param	yaw			yaw of entity.
 *	@param	pitch		pitch of entity.
 *	@param	roll		roll of entity.
 *	@param	data		entity's data.
 *
 *	type - index of python class stored in entities.xml
 *	state - really strange thing... I've tried to understood how data is stored, but have no luck. I guess it is a strange form of PyDict.
 **/
struct EntityCreatePacket : Packet
{
	TypeEntityId EntityId;
	TypeEntityType EntityType;
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
 *	@param	id				entity id.
 *	@param	messageID		message id.
 *	@param	data			message data.
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
 *	@param	id				entity id.
 *	@param	messageID		message id.
 *	@param	data			message data.
 **/
struct EntityPropertyPacket : Packet
{
	TypeEntityId EntityId;
	TypeMethodId MethodId;
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

/*
 * https://github.com/Monstrofil/replays_unpack/blob/parser2.0/docs/packets/0x22.md
 */
struct NestedPropertyUpdatePacket : Packet
{
	TypeEntityId EntityId;
	std::string PropertyName;
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

typedef std::variant<
	BasePlayerCreatePacket, CellPlayerCreatePacket, EntityControlPacket, EntityEnterPacket,
	EntityLeavePacket, EntityCreatePacket, EntityMethodPacket, EntityPropertyPacket,
	PlayerPositionPacket, PlayerOrientationPacket, MapPacket, NestedPropertyUpdatePacket,
	VersionPacket, CameraPacket, PlayerEntityPacket, UnknownPacket, InvalidPacket,
	CruiseStatePacket, CameraFreeLookPacket, CameraModePacket
> PacketType;

}  // namespace PotatoAlert::ReplayParser
