#include <std/core>
#include <std/mem>
#include <std/sys>

#pragma array_limit 999999
#pragma pattern_limit 9999999

bool doFilter in;
u32 packetFilter in;

#define DO_FILTER if (doFilter && type != packetFilter) continue

using EntityId = s32;
using SpaceId = s32;
using VehicleId = s32;

struct Vec3
{
    float x, y, z;
};

struct Vec4
{
    float w, x, y, z;
};

struct Mat4
{
    Vec4 data[4];
};

enum EntityType : u16
{
    Avatar = 1,
    Vehicle = 2,
    Account = 3,
    SmokeScreen = 4,
    OfflineEntity = 5,
    Login = 6,
    BattleEntity = 7,
    Building = 8,
    MasterChanger = 9,
    BattleLogic = 10,
    ReplayLeech = 11,
    ReplayConnectionHandler = 12,
    InteractiveZone = 13,
};

enum PacketType : u32
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
    Result               = 0x22,
    NestedPropertyUpdate = 0x23,
    Camera               = 0x25,  // 0x24
    CameraMode           = 0x27,  // 0x26
    Map                  = 0x28,
    PlayerOrientation    = 0x2C,  // 0x2B
    CameraFreeLook       = 0x2F,  // 0x2E
    CruiseState          = 0x32,  // 0x31

    Unknown_0x18         = 0x18,  // 52 bytes
    Unknown_0x1D         = 0x1D,  // 4 bytes
    Unknown_0x0E         = 0x0E,  // 8 bytes
    Unknown_0x0F         = 0x0F,  // 8 bytes - only once
    Unknown_0x10         = 0x10,  // 1 byte
    Unknown_0x13         = 0x13,  // 0 bytes - only once
    Unknown_0x26         = 0x26,  // 10 bytes
    Unknown_0x2A         = 0x2A,  // 32 bytes
    Unknown_0x30         = 0x30,  // 12 bytes
    Unknown_0xFFFFFFFF   = 0xFFFFFFFF,  // 16 bytes
};

struct BasePlayerCreate
{
    EntityId entityId;
    EntityType entityType;
    u32 valueSize;
    u8 values[valueSize];
};

struct CellPlayerCreate
{
    EntityId entityId;
    SpaceId spaceId;
    VehicleId vehicleId;
    Vec3 position;
    Vec3 rotation;
            
    u32 valueSize;
    u8 values[valueSize];
    //char unknown;
};

struct EntityControl
{
    EntityId entityId;
    bool isControlled;
};

struct EntityEnter
{
    EntityId entityId;
    SpaceId spaceId;
    VehicleId vehicleId;
};

struct EntityLeave
{
    EntityId entityId;
};

struct EntityCreate
{
    EntityId entityId;
    EntityType entityType;
    SpaceId spaceId;
    VehicleId vehicleId;
    Vec3 position;
    Vec3 rotation;

    u32 valueSize;
    u8 values[valueSize];
};

struct EntityMethod
{
    EntityId entityId;
    s32 methodId;
    u32 valueSize;
    u8 values[valueSize];
};

struct EntityProperty
{
    EntityId entityId;
    s32 propertyId;
    u32 valueSize;
    u8 values[valueSize];
};

struct PlayerOrientation
{
    EntityId entityId;
    VehicleId vehicleId;
    Vec3 position;
    Vec3 rotation;
};

struct PlayerPosition
{
    EntityId entityId;
    VehicleId vehicleId;
    Vec3 position;
    Vec3 positionError;
    Vec3 rotation;
    bool isError;
};

struct NestedPropertyUpdate
{
    EntityId entityId;
    bool isSlice;
    u32 valueSize;
    u8 values[valueSize];
};

struct Map
{
    SpaceId spaceId;
    s64 arenaId;
    u32 unknown1;
    u32 unknown2;
    u8 unknown3[128];
    u32 strSize;
    char name[strSize];
    Mat4 matrix;
    bool unknown4;
};

struct Camera
{
    Vec3 unknown1;
    float unknown2;
    Vec3 absolutePosition;
    float fov;
    Vec3 position;
    Vec3 rotation;
    float unknown3;
};
std::assert(sizeof(Camera) == 60, "sizeof(Camera) != 60");

struct Version
{
    u32 strSize;
    char version[strSize];
};

struct PlayerEntity
{
    EntityId entityId;
};

struct CruiseState
{
    u32 key;
    s32 value;
};

struct CameraFreeLook
{
    bool locked;
};

struct CameraMode
{
    u32 mode;
};

struct Result
{
    u32 strSize;
    char result[strSize];
};

struct Unknown_0x26
{
    EntityId entityId;
    u8 unknown[6];
};

struct Packet
{
    u32 size;
    PacketType type;
    float clock;
    //u8 packetData[size];
    
    match (type)
    {
        (PacketType::BasePlayerCreate):
        {
            BasePlayerCreate payload [[inline]];
            std::assert(size == sizeof(payload), "Invalid payload size for BasePlayerCreate");
            DO_FILTER;
        }
        (PacketType::CellPlayerCreate):
        {
            CellPlayerCreate payload [[inline]];
            std::assert(size == sizeof(payload), "Invalid payload size for CellPlayerCreate");
            DO_FILTER;
        }
        (PacketType::EntityControl):
        {
            EntityControl payload [[inline]];
            std::assert(size == sizeof(EntityControl), "Invalid payload size for EntityControl");
            DO_FILTER;
        }
        (PacketType::EntityEnter):
        {
            EntityEnter payload [[inline]];
            std::assert(size == sizeof(EntityEnter), "Invalid payload size for EntityEnter");
            DO_FILTER;
        }
        (PacketType::EntityLeave):
        {
            EntityLeave payload [[inline]];
            std::assert(size == sizeof(EntityLeave), "Invalid payload size for EntityLeave");
            DO_FILTER;
        }
        (PacketType::EntityCreate):
        {
            EntityCreate payload [[inline]];
            std::assert(size == sizeof(payload), "Invalid payload size for EntityCreate");
            DO_FILTER;
        }
        (PacketType::EntityProperty):
        {
            EntityProperty payload [[inline]];
            std::assert(size == sizeof(payload), "Invalid payload size for EntityProperty");
            DO_FILTER;
        }
        (PacketType::EntityMethod):
        {
            EntityMethod payload [[inline]];
            std::assert(size == sizeof(payload), "Invalid payload size for EntityMethod");
            DO_FILTER;
        }
        (PacketType::PlayerPosition):
        {
            PlayerPosition payload [[inline]];
            std::assert(size == sizeof(PlayerPosition), "Invalid payload size for PlayerPosition");
            DO_FILTER;
        }
        (PacketType::Version):
        {
            Version payload [[inline]];
            std::assert(size == sizeof(payload), "Invalid payload size for Version");
            DO_FILTER;
        }
        (PacketType::PlayerEntity):
        {
            PlayerEntity payload [[inline]];
            std::assert(size == sizeof(PlayerEntity), "Invalid payload size for PlayerEntity");
            DO_FILTER;
        }
        (PacketType::Result):
        {
            Result payload [[inline]];
            std::assert(size == sizeof(payload), "Invalid payload size for Result");
            DO_FILTER;
        }
        (PacketType::NestedPropertyUpdate):
        {
            NestedPropertyUpdate payload [[inline]];
            std::assert(size == sizeof(payload), "Invalid payload size for NestedPropertyUpdate");
            DO_FILTER;
        }
        (PacketType::Camera):
        {
            Camera payload [[inline]];
            std::assert(size == sizeof(Camera), "Invalid payload size for Camera");
            DO_FILTER;
        }
        (PacketType::CameraMode):
        {
            CameraMode payload [[inline]];
            std::assert(size == sizeof(CameraMode), "Invalid payload size for CameraMode");
            DO_FILTER;
        }
        (PacketType::Map):
        {
            Map payload [[inline]];
            std::assert(size == sizeof(payload), "Invalid payload size for Map");
            DO_FILTER;
        }
        (PacketType::PlayerOrientation):
        {
            PlayerOrientation payload [[inline]];
            std::assert(size == sizeof(PlayerOrientation), "Invalid payload size for PlayerOrientation");
            DO_FILTER;
        }
        (PacketType::CameraFreeLook):
        {
            CameraFreeLook payload [[inline]];
            std::assert(size == sizeof(CameraFreeLook), "Invalid payload size for CameraFreeLook");
            DO_FILTER;
        }
        (PacketType::CruiseState):
        {
            CruiseState payload [[inline]];
            std::assert(size == sizeof(CruiseState), "Invalid payload size for CruiseState");
            DO_FILTER;
        }

        // unknown packets
        (PacketType::Unknown_0x18):
        {
            u8 data[52];
            std::assert(size == sizeof(data), "Invalid payload size for Unknown_0x18");
            DO_FILTER;
        }
        (PacketType::Unknown_0x0E):
        {
            u8 data[8];
            std::assert(size == sizeof(data), "Invalid payload size for Unknown_0x0E");
            DO_FILTER;
        }
        (PacketType::Unknown_0x0F):
        {
            u8 data[8];
            std::assert(size == sizeof(data), "Invalid payload size for Unknown_0x0F");
            DO_FILTER;
        }
        (PacketType::Unknown_0x10):
        {
            u8 data[1];
            std::assert(size == sizeof(data), "Invalid payload size for Unknown_0x10");
            DO_FILTER;
        }
        (PacketType::Unknown_0x13):
        {
            std::assert(size == 0, "Invalid payload size for Unknown_0x13");
            DO_FILTER;
        }
        (PacketType::Unknown_0x1D):
        {
            u8 unknown[4];
            std::assert(size == sizeof(unknown), "Invalid payload size for Unknown_0x1D");
            DO_FILTER;
        }
        (PacketType::Unknown_0x26):
        {
            Unknown_0x26 payload [[inline]];
            std::assert(size == sizeof(payload), "Invalid payload size for Unknown_0x26");
            DO_FILTER;
        }
        (PacketType::Unknown_0x2A):
        {
            u8 data[32];
            std::assert(size == sizeof(data), "Invalid payload size for Unknown_0x2A");
            DO_FILTER;
        }
        (PacketType::Unknown_0x30):
        {
            u8 data[12];
            std::assert(size == sizeof(data), "Invalid payload size for Unknown_0x30");
            DO_FILTER;
        }
        (PacketType::Unknown_0xFFFFFFFF):
        {
            u8 data[16];
            std::assert(size == sizeof(data), "Invalid payload size for Unknown_0xFFFFFFFF");
            DO_FILTER;
        }
        (_):
        {
            u8 data[size];
        }
    }
};
Packet packets[while(!std::mem::eof())] @ 0x00;
