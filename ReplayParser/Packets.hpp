// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Math.hpp"

#include <optional>
#include <span>
#include <string>
#include <variant>
#include <vector>


using PotatoAlert::Vec3;

namespace PotatoAlert::ReplayParser {

enum class RibbonType
{
	PlaneShotDown,
	Incapacitation,
	SetFire,
	Citadel,
	SecondaryHit,
	OverPenetration,
	Penetration,
	NonPenetration,
	Ricochet,
	TorpedoProtectionHit,
	Captured,
	AssistedInCapture,
	Spotted,
	Destroyed,
	TorpedoHit,
	Defended,
	Flooding,
	DiveBombPenetration,
	RocketPenetration,
	RocketNonPenetration,
	RocketTorpedoProtectionHit,
	ShotDownByAircraft,
	Unknown
};

enum class DeathCause
{
	Secondaries,
	Artillery,
	Fire,
	Flooding,
	Torpedo,
	DiveBomber,
	AerialRocket,
	AerialTorpedo,
	Detonation,
	Ramming,
	Unknown
};

enum class VoiceLineType
{
	IntelRequired,
	FairWinds,
	Wilco,
	Negative,
	WellDone,
	Curses,
	UsingRadar,
	UsingHydroSearch,
	DefendTheBase,
	SetSmokeScreen,
	ProvideAntiAircraft,
	RequestingSupport,
	Retreat,
	AttentionToSquare,
	ConcentrateFire
};

enum class PacketType : uint32_t
{
	BasePlayerCreate = 0x0,
	CellPlayerCreate = 0x1,
	EntityControl = 0x2,
	EntityEnter = 0x3,
	EntityLeave = 0x4,
	EntityCreate = 0x5,
	EntityProperty = 0x7,
	EntityMethod = 0x8,
	Position = 0xA,
	NestedProperty = 0x22,
	Orientation = 0x2B
};

struct RawPacket
{
	uint32_t size;
	PacketType type;
	float clock;
	std::vector<std::byte> raw;

	static std::optional<RawPacket> FromBytes(std::span<std::byte>& data);
};

struct DecodedPacket
{
	PacketType type;
	float clock;
};

struct UnknownPacket : DecodedPacket
{
};

struct InvalidPacket : DecodedPacket
{
	std::string message;
	std::vector<std::byte> raw;
};

/*
 *	This function is called when we get a new player entity.
 *	The data on the stream contains only properties provided by the base.
 *
 *	@param	id			entity id.
 *	@param	type		entity type id.
 *	@param	data		entity data.
 */
struct BasePlayerCreatePacket : DecodedPacket
{
	uint32_t entityId;
	uint16_t entityType;
	uint32_t size;
	std::vector<std::byte> data;
};

/*
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
 */
struct CellPlayerCreatePacket : DecodedPacket
{
	uint32_t entityId;
	uint32_t spaceId;
	uint16_t unknown;
	uint32_t vehicleId;
	Vec3 position;
	Vec3 direction;  // yaw, pitch, roll
	uint32_t size;
	std::vector<std::byte> data;
};

/*
 *	This function is called to tell us that we may now control the given
 *	entity (or not as the case may be).
 *
 *	@param	id			entity id.
 *	@param	control		true if entity is now controlled, false otherwise.
 */
struct EntityControlPacket : DecodedPacket
{
	uint32_t entityId;
	bool isControlled;
};

/*
 *	This function is called when the server indicates that an entity has entered
 *	the player's AoI.
 *
 *	It is complicated because it may be called out of order.
 *
 *	@param	id			entity id.
 *	@param	spaceID		id of space where to create the entity in.
 *	@param	vehicleID	id of an entity to use as vehicle.
 *
 */
struct EntityEnterPacket : DecodedPacket
{
	uint32_t entityId;
	uint32_t spaceId;
	uint32_t vehicleId;
};

/*
 *	This function is called when the server indicates that an entity has left
 *	the player's AoI. It is complicated because it may be called out of order.
 *
 *	@param	id				entity id.
 *	@param	cacheStamps		Unused parameter.
 */
struct EntityLeavePacket : DecodedPacket
{
	uint32_t entityId;
};

/*
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
 */
struct EntityCreatePacket : DecodedPacket
{
	uint32_t entityId;
	uint16_t type;
	uint32_t spaceId;
	uint32_t vehicleId;
	Vec3 position;
	Vec3 direction;

	uint32_t stateSize;
	std::vector<std::byte> state;
};

/*
 *	This method is called when we receive a script method call message
 *	for one of our client-side entities from the server.
 *
 *	@param	id				entity id.
 *	@param	messageID		message id.
 *	@param	data			message data.
 */
struct EntityMethodPacket : DecodedPacket
{
	uint32_t entityId;
	uint32_t methodId;
	uint32_t size;
	std::vector<std::byte> data;

	static std::variant<EntityMethodPacket, InvalidPacket> Parse(float clock, std::span<std::byte>& data);
};

/*
 *	This method is called when we receive a property update message
 *	for one of our client-side entities from the server, aka server wants us to change a property on a client.
 *
 *	@param	id				entity id.
 *	@param	messageID		message id.
 *	@param	data			message data.
 */
struct EntityPropertyPacket : DecodedPacket
{
	uint32_t entityId;
	uint32_t methodId;
	uint32_t size;
	std::vector<std::byte> data;
};

/*
 * https://github.com/Monstrofil/replays_unpack/blob/parser2.0/docs/packets/0x22.md
 */
struct NestedPropertyPacket : DecodedPacket
{
	
};

/*
 * 
 */
struct PositionPacket : DecodedPacket
{
	uint32_t entityId;
	Vec3 position;
	Vec3 positionError;
	Vec3 rotation;
	bool isError;
};

struct CameraPacket : DecodedPacket
{
	uint32_t unknown1;
	uint32_t unknown2;
	uint32_t unknown3;
	uint32_t unknown4;
	uint32_t unknown5;
	uint32_t unknown6;
	uint32_t unknown7;

	uint32_t fov;
	Vec3 position;
	Vec3 direction;
};

/*
struct ChatPacket : DecodedPacket
{
	struct
	{
		uint32_t entityID;
		int32_t senderID;
		std::string audience;
		std::string message;
	} payload;

	static ChatPacket Parse(const RawPacket& packet);
};

struct VoiceLinePacket : DecodedPacket
{
	struct
	{
		uint32_t senderID;
		bool isGlobal;
		VoiceLineType message;
	} payload;
};

struct RibbonPacket : DecodedPacket
{
	struct
	{
		RibbonType ribbon;
	};
};
*/

struct BattleEndPacket : DecodedPacket
{
	int8_t winningTeam;
	uint8_t reason;
};

struct EntityPacket
{
	uint32_t superType;
	uint32_t entityId;
	uint32_t subType;
	std::vector<std::byte> payload;
};

struct PlayerOrientationPacket
{
	uint32_t pid;
	uint32_t parentId;

	Vec3 position;
	Vec3 direction;
};

typedef std::variant<
	BasePlayerCreatePacket, CellPlayerCreatePacket, EntityControlPacket, EntityEnterPacket,
	EntityLeavePacket, EntityCreatePacket, EntityMethodPacket, EntityPropertyPacket,
	UnknownPacket, InvalidPacket> Packet;

// TODO

}  // namespace DecodedPackets