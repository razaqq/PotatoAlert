#include <std/sys.pat>
#include <std/mem.pat>

#pragma pattern_limit 99999

struct Block
{
    u32 blockSize;
    if (blockSize > 0)
    {
        char data[blockSize];
    }
};

struct Replay
{
    char magic[4];
    u32 blockCount;
    u32 metaSize;
    char meta[metaSize];
    Block blocks[blockCount-1];
    u32 decompressedSize;
    u32 streamSize;
    
    u8 encryptedData[streamSize];

    u128 section = std::mem::create_section("Decrypted Data");
    std::mem::set_section_size(section, streamSize);

    str key = "\x29\xB7\xC9\x09\x38\x3F\x84\x88\xFA\x98\xEC\x4E\x13\x19\x79\xFB";

    std::assert(builtin::hex::crypt::decrypt(encryptedData, section, key, ""), "Failed to decrypt data");
    u8 decryptedData[streamSize] @ 0x00 in section;
    
    u128 decSection = std::mem::create_section("Decompressed Data");
    std::assert(builtin::hex::dec::zlib_decompress(decryptedData, decSection, 0), "Failed to decompress data");
    u8 decompressedData[decompressedSize] @ 0x00 in decSection;
};

Replay replay @ 0x00;

/*
#include <std/mem.pat>
#include <std/sys.pat>

#pragma pattern_limit 9999999

u8 decryptedData[std::mem::size()];

u128 decSection = std::mem::create_section("Decompressed Data");
std::assert(builtin::hex::dec::zlib_decompress(decryptedData, decSection, 0), "Failed to decompress data");
u8 decompressedData[decompressedSize] @ 0x00 in decSection;
*/
