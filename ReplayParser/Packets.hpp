// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Core/Math.hpp"
#include "GameFiles.hpp"
#include "Types.hpp"

#include <span>
#include <string>
#include <variant>
#include <vector>


using PotatoAlert::Vec3;

namespace PotatoAlert::ReplayParser {

/*
#if defined(__GNUC__) || defined(__clang__)
#define PACK(...) __VA_ARGS__ __attribute__((__packed__))
#elif defined(_MSC_VER)
#define PACK(...) __pragma("pack(push, 1)") __VA_ARGS__ __pragma("pack(pop)")
#else
#error Unsupported compiler!
#endif
*/

enum class PacketBaseType : uint32_t
{
	BasePlayerCreate = 0x0,
	CellPlayerCreate = 0x1,
	EntityControl = 0x2,
	EntityEnter = 0x3,
	EntityLeave = 0x4,
	EntityCreate = 0x5,
	EntityProperty = 0x7,
	EntityMethod = 0x8,
	PlayerPosition = 0xA,
	Version = 0x16,
	PlayerEntity = 0x20,
	NestedPropertyUpdate = 0x22,
	// Chat = 0x23,
	Camera = 0x24,
	Map = 0x27,
	PlayerOrientation = 0x2B
};

struct Packet
{
	PacketBaseType type;
	float clock;
};

struct UnknownPacket : Packet
{
};

struct InvalidPacket : Packet
{
	std::string message;
	std::vector<std::byte> raw;
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
	uint32_t entityId;
	uint16_t entityType;
	std::vector<std::byte> data;
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
	uint32_t entityId;
	uint32_t spaceId;
	uint16_t unknown;
	uint32_t vehicleId;
	Vec3 position;
	Rot3 rotation;
	std::unordered_map<std::string, ArgValue> values;
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
	uint32_t entityId;
	bool isControlled;
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
 *
 **/
struct EntityEnterPacket : Packet
{
	uint32_t entityId;
	uint32_t spaceId;
	uint32_t vehicleId;
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
	uint32_t entityId;
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
	uint32_t entityId;
	uint16_t entityType;
	uint32_t spaceId;
	uint32_t vehicleId;
	Vec3 position;
	Rot3 rotation;
	std::unordered_map<std::string, ArgValue> values;
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
	uint32_t entityId;
	uint32_t methodId;
	std::string methodName;
	std::vector<ArgValue> values;
	// uint32_t size;
	// std::vector<std::byte> data;
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
	uint32_t entityId;
	uint32_t methodId;
	// uint32_t size;
	// std::vector<std::byte> data;
	ArgValue value;
};

struct PlayerOrientationPacket : Packet
{
	uint32_t pid;
	uint32_t parentId;
	Vec3 position;
	Rot3 rotation;
};

struct PlayerPositionPacket : Packet
{
	uint32_t entityId;
	uint32_t vehicleId;
	Vec3 position;
	Vec3 positionError;
	Rot3 rotation;
	bool isError;
};

/*
 * https://github.com/Monstrofil/replays_unpack/blob/parser2.0/docs/packets/0x22.md
 */
struct NestedPropertyUpdatePacket : Packet
{
	uint32_t entityId;
	std::string propertyName;
};

struct MapPacket : Packet
{
	uint32_t spaceId;
	int64_t arenaId;
	uint32_t unknown1;
	uint32_t unknown2;
	std::string name;
	Mat4 matrix;
	bool unknown3;
};

struct CameraPacket : Packet
{
	Vec3 unknown;
	uint32_t unknown2;
	Vec3 absolutePosition;
	float fov;
	Vec3 position;
	Rot3 rotation;
};

struct VersionPacket : Packet
{
	std::string version;
};

struct PlayerEntityPacket : Packet
{
	uint32_t entityId;
};

typedef std::variant<
	BasePlayerCreatePacket, CellPlayerCreatePacket, EntityControlPacket, EntityEnterPacket,
	EntityLeavePacket, EntityCreatePacket, EntityMethodPacket, EntityPropertyPacket,
	PlayerPositionPacket, PlayerOrientationPacket, MapPacket, NestedPropertyUpdatePacket,
	VersionPacket, CameraPacket, PlayerEntityPacket, UnknownPacket, InvalidPacket> PacketType;

}  // namespace PotatoAlert::ReplayParser
