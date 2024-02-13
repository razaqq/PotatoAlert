// Copyright 2024 <github.com/razaqq>

#include <std/io.pat>
#include <std/ptr.pat>
#include <std/hash.pat>
#include <std/mem.pat>
#include <type/magic.pat>


struct IdxHeader
{
    type::Magic<"ISFP"> Signature;
        
    u32 Endianness;
    if (Endianness != 0x2000000)
        std::error("Invalid endianness");
    
    u32 MurmurHash;    
    
    u32 Version;
    if (Version != 0x40)
        std::error("Invalid version");
        
    u32 NodeCount;
    u32 FileCount;
    u32 VolumeCount;
    
    u32 Unknown;  // Unused?
    
    u64 NodeTablePtr;
    u64 FileRecordPtr;
    u64 VolumeTablePtr;
    
    std::assert(sizeof(this) == 0x38, "Invalid IdxHeader size");
};

struct String
{
    char value[];
} [[sealed, format("format_string")]];

fn format_string(ref String s)
{
    return s.value;
};

struct IdxNode
{
    u64 NameLength;
    u64 NamePtr;
    u64 Id;
    u64 Parent;
    String Name @ addressof(this) + NamePtr;

    std::assert(sizeof(Name.value) == NameLength, "Invalid node name length");
    std::assert(sizeof(this) == 0x20, "Invalid IdxNode size");
};

struct IdxFileRecord
{
    u64 NodeId;
    u64 VolumeId;
    u64 Offset;
    u64 CompressionInfo;
    u32 Size;
    u32 Crc32;
    u32 UncompressedSize;
    u32 Padding;

    std::assert(sizeof(this) == 0x30, "Invalid IdxFileRecord size");
};

struct IdxVolume
{
    u64 NameLength;
    u64 NamePtr;
    u64 Id;
    String Name @ addressof(this) + NamePtr;
 
    std::assert(sizeof(Name.value) == NameLength, "Invalid volume name length");
    std::assert(sizeof(this) == 0x18, "Invalid IdxVolume size");
};

struct IdxFile
{
    IdxHeader Header;
    IdxNode Nodes[Header.NodeCount] @ Header.NodeTablePtr + 0x10;
    IdxFileRecord FileRecords[Header.FileCount] @ Header.FileRecordPtr + 0x10;
    IdxVolume Volumes[Header.VolumeCount] @ Header.VolumeTablePtr + 0x10;
};

IdxFile idxFile @ 0x00;

fn assert_valid(ref IdxFile f)
{
    for (u32 i = 0, i < f.Header.FileCount, i = i + 1)
    {
        bool found = false;
        for (u32 j = 0, j < f.Header.NodeCount, j = j + 1)
        {
            if (f.Nodes[j].Id == f.FileRecords[i].NodeId)
            {
                found = true;
                break;
            }
        }
        
        if (!found)
        {
            return false;
        }
    }
    
    return true;
};

std::assert(assert_valid(idxFile), "IdxFile was invalid");