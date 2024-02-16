// Copyright 2024 <github.com/razaqq>
#pragma once

#include "Core/Bytes.hpp"
#include "Core/Result.hpp"

#include <bitset>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <variant>
#include <vector>


namespace PotatoAlert::Core {

enum class PeError
{
	InvalidMagic,
	InvalidDosHeader,
	InvalidPeHeader,
	InvalidCoffHeaderPtr,
	InvalidNtHeader,
	InvalidImageFileHeader,
	InvalidOptionalHeader32,
	InvalidOptionalHeader64,
	InvalidPeFormat,
	InvalidSectionHeader,
	InvalidSectionHeaders,
	InvalidResourceSection,
	InvalidResourceSectionTypeEntry,
	InvalidResourceSectionNameEntry,
	InvalidResourceSectionLangEntry,
	InvalidResourceSectionDataEntry,
	InvalidImageResourceDirectoryEntry,
	InvalidVersionInfo,
};

std::string_view GetErrorMessage(PeError error);

static constexpr uint32_t DosHeaderSize = 0x40;
struct DosHeader
{
	uint16_t ExtraPageSize;
	uint16_t NumberOfPages;
	uint16_t Relocations;
	uint16_t HeaderSizeInParagraphs;
	uint16_t MinimumAllocatedParagraphs;
	uint16_t MaximumAllocatedParagraphs;
	uint16_t InitialSSValue;
	uint16_t InitialRelativeSPValue;
	uint16_t Checksum;
	uint16_t InitialRelativeIPValue;
	uint16_t InitialCSValue;
	uint16_t RelocationsTablePointer;
	uint16_t OverlayNumber;
	uint16_t ReservedWords[4];
	uint16_t OemIdentifier;
	uint16_t OemInformation;
	uint16_t OtherReservedWords[10];
	uint32_t CoffHeaderPointer;

	static Result<DosHeader, PeError> FromData(std::span<const Byte> data);
};

struct DosStub
{
	std::vector<uint8_t> Code;
	std::string Message;
	std::string Data;

	static Result<DosStub, PeError> FromData(std::span<const Byte> data);
};

struct PeHeader
{
	DosHeader DosHeader;
	DosStub DosStub;

	static Result<PeHeader, PeError> FromData(std::span<const Byte> data);
};

struct ImageDataDirectory
{
	uint32_t VirtualAddress;
	uint32_t Size;
};
static_assert(sizeof(ImageDataDirectory) == 0x08);

enum class PeFormat : uint16_t
{
	Rom = 0x107,
	Pe32 = 0x10B,
	Pe32Plus = 0x20B,
};

static constexpr int ImageNumberOfDirectoryEntries = 16;

static constexpr uint32_t OptionalHeader32FixedSize = 0x60;  // size of the all members except Directories
struct OptionalHeader32
{
	PeFormat Magic;
	Byte     MajorLinkerVersion;
	Byte     MinorLinkerVersion;
	uint32_t SizeOfCode;
	uint32_t SizeOfInitializedData;
	uint32_t SizeOfUninitializedData;
	uint32_t AddressOfEntryPoint;
	uint32_t BaseOfCode;
	uint32_t BaseOfData;
	uint32_t ImageBase;
	uint32_t SectionAlignment;
	uint32_t FileAlignment;
	uint16_t MajorOperatingSystemVersion;
	uint16_t MinorOperatingSystemVersion;
	uint16_t MajorImageVersion;
	uint16_t MinorImageVersion;
	uint16_t MajorSubsystemVersion;
	uint16_t MinorSubsystemVersion;
	uint32_t Win32VersionValue;
	uint32_t SizeOfImage;
	uint32_t SizeOfHeaders;
	uint32_t CheckSum;
	uint16_t Subsystem;
	uint16_t DllCharacteristics;
	uint32_t SizeOfStackReserve;
	uint32_t SizeOfStackCommit;
	uint32_t SizeOfHeapReserve;
	uint32_t SizeOfHeapCommit;
	uint32_t LoaderFlags;
	uint32_t NumberOfRvaAndSizes;
	ImageDataDirectory Directories[ImageNumberOfDirectoryEntries];

	static Result<OptionalHeader32, PeError> FromData(PeFormat f, std::span<const Byte>& data);
};

static constexpr uint32_t OptionalHeader64FixedSize = 0x70;  // size of the all members except Directories
struct OptionalHeader64
{
	PeFormat Magic;
	Byte     MajorLinkerVersion;
	Byte     MinorLinkerVersion;
	uint32_t SizeOfCode;
	uint32_t SizeOfInitializedData;
	uint32_t SizeOfUninitializedData;
	uint32_t AddressOfEntryPoint;
	uint32_t BaseOfCode;
	uint64_t ImageBase;
	uint32_t SectionAlignment;
	uint32_t FileAlignment;
	uint16_t MajorOperatingSystemVersion;
	uint16_t MinorOperatingSystemVersion;
	uint16_t MajorImageVersion;
	uint16_t MinorImageVersion;
	uint16_t MajorSubsystemVersion;
	uint16_t MinorSubsystemVersion;
	uint32_t Win32VersionValue;
	uint32_t SizeOfImage;
	uint32_t SizeOfHeaders;
	uint32_t CheckSum;
	uint16_t Subsystem;
	uint16_t DllCharacteristics;
	uint64_t SizeOfStackReserve;
	uint64_t SizeOfStackCommit;
	uint64_t SizeOfHeapReserve;
	uint64_t SizeOfHeapCommit;
	uint32_t LoaderFlags;
	uint32_t NumberOfRvaAndSizes;
	ImageDataDirectory Directories[ImageNumberOfDirectoryEntries];

	static Result<OptionalHeader64, PeError> FromData(PeFormat f, std::span<const Byte>& data);
};

enum ArchitectureType : uint16_t
{
	Unknown = 0x00,
	ALPHAAXPOld = 0x183,
	ALPHAAXP = 0x184,
	ALPHAAXP64Bit = 0x284,
	AM33 = 0x1D3,
	AMD64 = 0x8664,
	ARM = 0x1C0,
	ARM64 = 0xAA64,
	ARMNT = 0x1C4,
	CLRPureMSIL = 0xC0EE,
	EBC = 0xEBC,
	I386 = 0x14C,
	I860 = 0x14D,
	IA64 = 0x200,
	LOONGARCH32 = 0x6232,
	LOONGARCH64 = 0x6264,
	M32R = 0x9041,
	MIPS16 = 0x266,
	MIPSFPU = 0x366,
	MIPSFPU16 = 0x466,
	MOTOROLA68000 = 0x268,
	POWERPC = 0x1F0,
	POWERPCFP = 0x1F1,
	POWERPC64 = 0x1F2,
	R3000 = 0x162,
	R4000 = 0x166,
	R10000 = 0x168,
	RISCV32 = 0x5032,
	RISCV64 = 0x5064,
	RISCV128 = 0x5128,
	SH3 = 0x1A2,
	SH3DSP = 0x1A3,
	SH4 = 0x1A6,
	SH5 = 0x1A8,
	THUMB = 0x1C2,
	WCEMIPSV2 = 0x169
};

struct CharacteristicsType
{
	Byte BaseRelocationsStripped : 1;
	Byte ExecutableImage : 1;
	Byte LineNumbersStripped : 1;
	Byte SymbolsStripped : 1;
	Byte AggressivelyTrimWorkingSet : 1;
	Byte LargeAddressAware : 1;
	Byte Padding : 1;
	Byte BytesReversedLo : 1;
	Byte Machine32Bit : 1;
	Byte DebugInfoStripped : 1;
	Byte RemovableRunFromSwap : 1;
	Byte NetRunFromSwap : 1;
	Byte SystemFile : 1;
	Byte Dll : 1;
	Byte UniprocessorMachineOnly : 1;
	Byte BytesReversedHi : 1;
};
static_assert(sizeof(CharacteristicsType) == sizeof(uint16_t));

static constexpr uint32_t ImageFileHeaderSize = 0x14;
struct ImageFileHeader
{
	ArchitectureType Architecture;
	uint16_t NumberOfSections;
	uint32_t TimeDateStamp;
	uint32_t PointerToSymbolTable;
	uint32_t NumberOfSymbols;
	uint16_t SizeOfOptionalHeader;
	CharacteristicsType Characteristics;

	static Result<ImageFileHeader, PeError> FromData(std::span<const Byte>& data);

};
static_assert(sizeof(ImageFileHeader) == ImageFileHeaderSize);

struct NtHeader
{
	ImageFileHeader ImageFileHeader;
	std::variant<OptionalHeader32, OptionalHeader64> OptionalHeader;

	static Result<NtHeader, PeError> FromData(std::span<const Byte>& data);
};

enum class ImageDirectoryEntryType
{
	Export = 0,
	Import = 1,
	Resource = 2,
	Exception = 3,
	Security = 4,
	BaseReloc = 5,
	Debug = 6,
	Copyright = 7,
	GlobalPtr = 8,
	Tls = 9,
	LoadConfig = 10,
};

static constexpr uint32_t SectionHeaderSize = 0x28;
struct SectionHeader
{
	char Name[8];
	union {
		uint32_t PhysicalAddress;
		uint32_t VirtualSize;
	};
	uint32_t VirtualAddress;
	uint32_t SizeOfRawData;
	uint32_t PtrRawData;
	uint32_t PtrRelocations;
	uint32_t PtrLineNumbers;
	uint16_t NumberOfRelocations;
	uint16_t NumberOfLineNumbers;
	uint32_t Characteristics;

	static Result<SectionHeader, PeError> FromData(std::span<const Byte> data);
};

// https://learn.microsoft.com/en-us/windows/win32/api/verrsrc/ns-verrsrc-vs_fixedfileinfo
struct VsFixedFileInfo
{
	uint32_t Signature; /* e.g. 0xfeef04bd */
	uint32_t StrucVersion; /* e.g. 0x00000042 = "0.42" */
	uint32_t FileVersionMS; /* e.g. 0x00030075 = "3.75" */
	uint32_t FileVersionLS; /* e.g. 0x00000031 = "0.31" */
	uint32_t ProductVersionMS; /* e.g. 0x00030010 = "3.10" */
	uint32_t ProductVersionLS; /* e.g. 0x00000031 = "0.31" */
	uint32_t FileFlagsMask; /* = 0x3F for version "0.42" */
	uint32_t FileFlags; /* e.g. VFF_DEBUG | VFF_PRERELEASE */
	uint32_t FileOS; /* e.g. VOS_DOS_WINDOWS16 */
	uint32_t FileType; /* e.g. VFT_DRIVER */
	uint32_t FileSubtype; /* e.g. VFT2_DRV_KEYBOARD */
	uint32_t FileDateMS; /* e.g. 0 */
	uint32_t FileDateLS; /* e.g. 0 */
};
static_assert(sizeof(VsFixedFileInfo) == 0x34);

// https://learn.microsoft.com/en-us/windows/win32/menurc/vs-versioninfo
struct VsVersionInfo
{
	uint16_t Length;
	uint16_t ValueLength;
	uint16_t Type;
	char16_t Key[16];
	alignas(4) VsFixedFileInfo Value;
	alignas(4) uint16_t Children;  // TODO: this is a var length array

	static Result<VsVersionInfo, PeError> FromData(std::span<const Byte> data);
};
static_assert(offsetof(VsVersionInfo, Value) % 4 == 0);

enum class ResourceType
{
	Cursor = 1,
	Bitmap = 2,
	Icon  = 3,
	Menu = 4,
	Dialog = 5,
	String = 6,
	FontDir = 7,
	Font = 8,
	Accelerator = 9,
	RcData  = 10,
	MessageTable = 11,
	GroupCursor = 12,
	GroupIcon = 14,
	Version = 16,
	DlgInclude = 17,
	PlugPlay = 19,
	Vdx = 20,
	AniCursor = 21,
	AniIcon = 22,
	Html = 23,
	Manifest = 24,
};

struct Resource
{
	ResourceType Type;
	std::variant<std::wstring, uint32_t> Id;
	uint32_t LanguageId;
	std::span<const Byte> Data;
};

struct ResourceTable
{
	std::vector<Resource> Resources;

	static Result<ResourceTable, PeError> FromData(std::span<const Byte> data, std::span<const Byte> fullData, uint32_t rva);
};

class PeReader
{
public:
	explicit PeReader(std::span<const Byte> data) : m_data(data)
	{

	}

	Result<void, PeError> Parse();

	const std::optional<PeHeader>& GetDosHeader() const
	{
		return m_peHeader;
	}
	const std::optional<NtHeader>& GetNtHeader() const
	{
		return m_ntHeader;
	}
	const std::vector<SectionHeader>& GetSectionHeaders() const
	{
		return m_sectionHeaders;
	}
	const std::optional<ResourceTable>& GetResourceTable() const
	{
		return m_table;
	}

private:
	std::span<const Byte> m_data;
	std::optional<PeHeader> m_peHeader;
	std::optional<NtHeader> m_ntHeader;
	std::vector<SectionHeader> m_sectionHeaders;
	std::optional<ResourceTable> m_table;
};

}  // namespace PotatoAlert::Core
