// Copyright 2024 <github.com/razaqq>

#include "Core/Bytes.hpp"
#include "Core/FileMagic.hpp"
#include "Core/PeReader.hpp"
#include "Core/Result.hpp"
#include "Core/TypeTraits.hpp"
#include "Core/Version.hpp"

#include <bitset>
#include <cstdint>
#include <string>
#include <span>


using PotatoAlert::Core::Byte;
using PotatoAlert::Core::DosHeader;
using PotatoAlert::Core::DosStub;
using PotatoAlert::Core::ImageFileHeader;
using PotatoAlert::Core::NtHeader;
using PotatoAlert::Core::OptionalHeader32;
using PotatoAlert::Core::OptionalHeader64;
using PotatoAlert::Core::PeHeader;
using PotatoAlert::Core::PeReader;
using PotatoAlert::Core::PeError;
using PotatoAlert::Core::Result;
using PotatoAlert::Core::ResourceTable;
using PotatoAlert::Core::SectionHeader;
using PotatoAlert::Core::TakeInto;
using PotatoAlert::Core::VsVersionInfo;
using PotatoAlert::Core::Version;

namespace {

struct ImageResourceDirectory
{
	uint32_t Characteristics;
	uint32_t TimeDateStamp;
	uint16_t MajorVersion;
	uint16_t MinorVersion;
	uint16_t NumberOfNamedEntries;
	uint16_t NumberOfIdEntries;
};
static_assert(sizeof(ImageResourceDirectory) == 0x10);

struct ImageResourceDataEntry
{
	uint32_t OffsetToData;  // relative virtual offset
	uint32_t Size;
	uint32_t CodePage;
	uint32_t Reserved;
};
static_assert(sizeof(ImageResourceDataEntry) == 0x10);

struct ImageResourceDirectoryEntry
{
	uint32_t Name;  // relative to root node
	std::bitset<32> OffsetToData;  // relative to root node

	[[nodiscard]] uint32_t GetOffset() const
	{
		return OffsetToData.to_ulong() & ~(1ul << 31ul);
	}

	[[nodiscard]] bool GetBit() const
	{
		return OffsetToData.to_ulong() & 1ul << 31ul;
	}

	static Result<ImageResourceDirectoryEntry, PeError> FromData(std::span<const Byte>& data)
	{
		if (data.size() < 8)
			return PA_ERROR(PeError::InvalidImageResourceDirectoryEntry);

		ImageResourceDirectoryEntry e;
		TakeInto(data, e.Name);
		uint32_t offset;
		TakeInto(data, offset);
		e.OffsetToData = offset;

		return e;
	}
};

}

std::string_view PotatoAlert::Core::GetErrorMessage(PeError error)
{
	switch (error)
	{
		case PeError::InvalidMagic:
			return "Invalid Magic";
		case PeError::InvalidDosHeader:
			return "Invalid DOS Header";
		case PeError::InvalidPeHeader:
			return "Invalid PE Header";
		case PeError::InvalidCoffHeaderPtr:
			return "Invalid Coff Header Pointer";
		case PeError::InvalidNtHeader:
			return "Invalid NT Header";
		case PeError::InvalidImageFileHeader:
			return "Invalid Image File Header";
		case PeError::InvalidOptionalHeader32:
			return "Invalid OptionalHeader32";
		case PeError::InvalidOptionalHeader64:
			return "Invalid OptionalHeader64";
		case PeError::InvalidPeFormat:
			return "Invalid PE Format";
		case PeError::InvalidSectionHeader:
			return "Invalid Section Header";
		case PeError::InvalidSectionHeaders:
			return "Invalid Section Headers";
		case PeError::InvalidResourceSection:
			return "Invalid Resource Sections";
		case PeError::InvalidResourceSectionTypeEntry:
			return "Invalid Resource Section Type Entry";
		case PeError::InvalidResourceSectionNameEntry:
			return "Invalid Resource Section Name Entry";
		case PeError::InvalidResourceSectionLangEntry:
			return "Invalid Resource Section Language Entry";
		case PeError::InvalidResourceSectionDataEntry:
			return "Invalid Resource Section Data Entry";
		case PeError::InvalidImageResourceDirectoryEntry:
			return "Invalid ImageResourceDirectoryEntry";
		case PeError::InvalidVersionInfo:
			break;
	}
	return "Unknown Error";
}

Result<DosHeader, PeError> DosHeader::FromData(std::span<const Byte> data)
{
	if (data.size() != DosHeaderSize)
	{
		return PA_ERROR(PeError::InvalidDosHeader);
	}

	if (!FileMagic<'M', 'Z'>(data))
	{
		return PA_ERROR(PeError::InvalidMagic);
	}

	DosHeader h;
	TakeInto(data, h.ExtraPageSize);
	TakeInto(data, h.NumberOfPages);
	TakeInto(data, h.Relocations);
	TakeInto(data, h.HeaderSizeInParagraphs);
	TakeInto(data, h.MinimumAllocatedParagraphs);
	TakeInto(data, h.MaximumAllocatedParagraphs);
	TakeInto(data, h.InitialSSValue);
	TakeInto(data, h.InitialRelativeSPValue);
	TakeInto(data, h.Checksum);
	TakeInto(data, h.InitialRelativeIPValue);
	TakeInto(data, h.InitialCSValue);
	TakeInto(data, h.RelocationsTablePointer);
	TakeInto(data, h.OverlayNumber);
	TakeInto(data, h.ReservedWords);
	TakeInto(data, h.OemIdentifier);
	TakeInto(data, h.OemInformation);
	TakeInto(data, h.OtherReservedWords);
	TakeInto(data, h.CoffHeaderPointer);

	return h;
}

Result<DosStub, PeError> DosStub::FromData(std::span<const Byte> data)
{
	// TODO
	return {};
}

Result<PeHeader, PeError> PeHeader::FromData(std::span<const Byte> data)
{
	if (data.size() < DosHeaderSize)
		return PA_ERROR(PeError::InvalidPeHeader);

	PeHeader h;
	PA_TRYA(h.DosHeader, DosHeader::FromData(Take(data, DosHeaderSize)));
	PA_TRYA(h.DosStub, DosStub::FromData(data));
	
	return h;
}

Result<OptionalHeader32, PeError> OptionalHeader32::FromData(PeFormat f, std::span<const Byte>& data)
{
	if (data.size() < OptionalHeader32FixedSize - sizeof(PeFormat))
	{
		return PA_ERROR(PeError::InvalidOptionalHeader32);
	}

	OptionalHeader32 h;
	h.Magic = f;
	TakeInto(data, h.MajorLinkerVersion);
	TakeInto(data, h.MinorLinkerVersion);
	TakeInto(data, h.SizeOfCode);
	TakeInto(data, h.SizeOfInitializedData);
	TakeInto(data, h.SizeOfUninitializedData);
	TakeInto(data, h.AddressOfEntryPoint);
	TakeInto(data, h.BaseOfCode);
	TakeInto(data, h.BaseOfData);
	TakeInto(data, h.ImageBase);
	TakeInto(data, h.SectionAlignment);
	TakeInto(data, h.FileAlignment);
	TakeInto(data, h.MajorOperatingSystemVersion);
	TakeInto(data, h.MinorOperatingSystemVersion);
	TakeInto(data, h.MajorImageVersion);
	TakeInto(data, h.MinorImageVersion);
	TakeInto(data, h.MajorSubsystemVersion);
	TakeInto(data, h.MinorSubsystemVersion);
	TakeInto(data, h.Win32VersionValue);
	TakeInto(data, h.SizeOfImage);
	TakeInto(data, h.SizeOfHeaders);
	TakeInto(data, h.CheckSum);
	TakeInto(data, h.Subsystem);
	TakeInto(data, h.DllCharacteristics);
	TakeInto(data, h.SizeOfStackReserve);
	TakeInto(data, h.SizeOfStackCommit);
	TakeInto(data, h.SizeOfHeapReserve);
	TakeInto(data, h.SizeOfHeapCommit);
	TakeInto(data, h.LoaderFlags);
	TakeInto(data, h.NumberOfRvaAndSizes);

	if (data.size() != h.NumberOfRvaAndSizes * sizeof(ImageDataDirectory))
	{
		return PA_ERROR(PeError::InvalidOptionalHeader32);
	}

	for (uint32_t i = 0; i < h.NumberOfRvaAndSizes; i++)
	{
		ImageDataDirectory d;
		TakeInto(data, d);
		h.Directories[i] = d;
	}

	if (!data.empty())
	{
		return PA_ERROR(PeError::InvalidOptionalHeader32);
	}

	return h;
}

Result<OptionalHeader64, PeError> OptionalHeader64::FromData(PeFormat f, std::span<const Byte>& data)
{
	if (data.size() < OptionalHeader64FixedSize - sizeof(PeFormat))
	{
		return PA_ERROR(PeError::InvalidOptionalHeader32);
	}

	OptionalHeader64 h;
	h.Magic = f;
	TakeInto(data, h.MajorLinkerVersion);
	TakeInto(data, h.MinorLinkerVersion);
	TakeInto(data, h.SizeOfCode);
	TakeInto(data, h.SizeOfInitializedData);
	TakeInto(data, h.SizeOfUninitializedData);
	TakeInto(data, h.AddressOfEntryPoint);
	TakeInto(data, h.BaseOfCode);
	TakeInto(data, h.ImageBase);
	TakeInto(data, h.SectionAlignment);
	TakeInto(data, h.FileAlignment);
	TakeInto(data, h.MajorOperatingSystemVersion);
	TakeInto(data, h.MinorOperatingSystemVersion);
	TakeInto(data, h.MajorImageVersion);
	TakeInto(data, h.MinorImageVersion);
	TakeInto(data, h.MajorSubsystemVersion);
	TakeInto(data, h.MinorSubsystemVersion);
	TakeInto(data, h.Win32VersionValue);
	TakeInto(data, h.SizeOfImage);
	TakeInto(data, h.SizeOfHeaders);
	TakeInto(data, h.CheckSum);
	TakeInto(data, h.Subsystem);
	TakeInto(data, h.DllCharacteristics);
	TakeInto(data, h.SizeOfStackReserve);
	TakeInto(data, h.SizeOfStackCommit);
	TakeInto(data, h.SizeOfHeapReserve);
	TakeInto(data, h.SizeOfHeapCommit);
	TakeInto(data, h.LoaderFlags);
	TakeInto(data, h.NumberOfRvaAndSizes);

	if (data.size() != h.NumberOfRvaAndSizes * sizeof(ImageDataDirectory))
	{
		return PA_ERROR(PeError::InvalidOptionalHeader64);
	}

	for (uint32_t i = 0; i < h.NumberOfRvaAndSizes; i++)
	{
		ImageDataDirectory d;
		TakeInto(data, d);
		h.Directories[i] = d;
	}

	if (!data.empty())
	{
		return PA_ERROR(PeError::InvalidOptionalHeader64);
	}

	return h;
}

Result<ImageFileHeader, PeError> ImageFileHeader::FromData(std::span<const Byte>& data)
{
	if (data.size() < ImageFileHeaderSize)
	{
		return PA_ERROR(PeError::InvalidImageFileHeader);
	}

	ImageFileHeader h;
	TakeInto(data, h.Architecture);
	TakeInto(data, h.NumberOfSections);
	TakeInto(data, h.TimeDateStamp);
	TakeInto(data, h.PointerToSymbolTable);
	TakeInto(data, h.NumberOfSymbols);
	TakeInto(data, h.SizeOfOptionalHeader);
	TakeInto(data, h.Characteristics);

	return h;
}

Result<NtHeader, PeError> NtHeader::FromData(std::span<const Byte>& data)
{
	if (!FileMagic<'P', 'E', '\0', '\0'>(data))
	{
		return PA_ERROR(PeError::InvalidMagic);
	}

	NtHeader h;
	PA_TRYA(h.ImageFileHeader, ImageFileHeader::FromData(data));

	if (data.size() < h.ImageFileHeader.SizeOfOptionalHeader)
	{
		return PA_ERROR(PeError::InvalidNtHeader);
	}
	auto optData = Take(data, h.ImageFileHeader.SizeOfOptionalHeader);

	PeFormat peFormat;
	if (!TakeInto(optData, peFormat))
		return PA_ERROR(PeError::InvalidNtHeader);

	switch (peFormat)
	{
		case PeFormat::Rom:
		case PeFormat::Pe32:
		{
			PA_TRYA(h.OptionalHeader, OptionalHeader32::FromData(peFormat, optData));
			break;
		}
		case PeFormat::Pe32Plus:
		{
			PA_TRYA(h.OptionalHeader, OptionalHeader64::FromData(peFormat, optData));
			break;
		}
		default:
			return PA_ERROR(PeError::InvalidPeFormat);
	}

	return h;
}

Result<SectionHeader, PeError> SectionHeader::FromData(std::span<const Byte> data)
{
	if (data.size() != SectionHeaderSize)
	{
		return PA_ERROR(PeError::InvalidSectionHeader);
	}

	SectionHeader h;
	TakeInto(data, h.Name);
	TakeInto(data, h.VirtualSize);
	TakeInto(data, h.VirtualAddress);
	TakeInto(data, h.SizeOfRawData);
	TakeInto(data, h.PtrRawData);
	TakeInto(data, h.PtrRelocations);
	TakeInto(data, h.PtrLineNumbers);
	TakeInto(data, h.NumberOfRelocations);
	TakeInto(data, h.NumberOfLineNumbers);
	TakeInto(data, h.Characteristics);
	return h;
}

Result<VsVersionInfo, PeError> VsVersionInfo::FromData(std::span<const Byte> data)
{
	if (data.size() < sizeof(VsVersionInfo::Length))
	{
		return PA_ERROR(PeError::InvalidVersionInfo);
	}

	const size_t size = data.size();

	VsVersionInfo info;
	TakeInto(data, info.Length);

	if (size != info.Length)
	{
		return PA_ERROR(PeError::InvalidVersionInfo);
	}

	if (!TakeInto(data, info.ValueLength))
	{
		return PA_ERROR(PeError::InvalidVersionInfo);
	}

	if (info.ValueLength != sizeof(VsFixedFileInfo))
	{
		return PA_ERROR(PeError::InvalidVersionInfo);
	}

	if (!TakeInto(data, info.Type))
	{
		return PA_ERROR(PeError::InvalidVersionInfo);
	}
	if (!TakeInto(data, info.Key))
	{
		return PA_ERROR(PeError::InvalidVersionInfo);
	}

	if (info.Key != std::u16string_view(u"VS_VERSION_INFO"))
	{
		return PA_ERROR(PeError::InvalidVersionInfo);
	}

	constexpr size_t padding1Size = offsetof(VsVersionInfo, Value) - offsetof(VsVersionInfo, Key) - 16 * sizeof(char16_t);
	if (data.size() < padding1Size)
	{
		return PA_ERROR(PeError::InvalidVersionInfo);
	}
	Take(data, padding1Size);
	if (!TakeInto(data, info.Value))
	{
		return PA_ERROR(PeError::InvalidVersionInfo);
	}
	constexpr size_t padding2Size = offsetof(VsVersionInfo, Children) - offsetof(VsVersionInfo, Value) - sizeof(VsFixedFileInfo);
	if (data.size() < padding2Size)
	{
		return PA_ERROR(PeError::InvalidVersionInfo);
	}
	Take(data, padding2Size);
	if (!TakeInto(data, info.Children))
	{
		return PA_ERROR(PeError::InvalidVersionInfo);
	}

	return info;
}

Result<ResourceTable, PeError> ResourceTable::FromData(std::span<const Byte> data, std::span<const Byte> fullData, uint32_t rva)
{
	std::span<const Byte> rtData = data;

	ResourceTable resourceTable;

	ImageResourceDirectory typeRoot;
	if (!TakeInto(data, typeRoot))
	{
		return PA_ERROR(PeError::InvalidResourceSection);
	}

	if (typeRoot.NumberOfNamedEntries != 0)
	{
		return PA_ERROR(PeError::InvalidResourceSection);
	}

	for (uint16_t i = 0; i < typeRoot.NumberOfIdEntries; i++)
	{
		PA_TRY(typeEntry, ImageResourceDirectoryEntry::FromData(data));

		const ResourceType resourceType = static_cast<ResourceType>(typeEntry.Name);

		if (!typeEntry.GetBit())  // these should all be dirs
		{
			return PA_ERROR(PeError::InvalidResourceSectionTypeEntry);
		}

		const uint32_t nameOffset = typeEntry.GetOffset();
		if (nameOffset >= data.size())
		{
			return PA_ERROR(PeError::InvalidResourceSectionNameEntry);
		}
		auto nameData = rtData.subspan(nameOffset, rtData.size() - nameOffset);

		ImageResourceDirectory nameRoot;
		if (!TakeInto(nameData, nameRoot))
		{
			return PA_ERROR(PeError::InvalidResourceSectionNameEntry);
		}

		// first named then id
		// first bit of id decides aswell
		for (uint16_t j = 0; j < nameRoot.NumberOfNamedEntries + nameRoot.NumberOfIdEntries; j++)
		{
			Resource resource;
			resource.Type = resourceType;

			PA_TRY(nameEntry, ImageResourceDirectoryEntry::FromData(nameData));

			if (nameEntry.Name & 1u << 31u)
			{
				uint32_t offset = nameEntry.Name & ~(1u << 31u);
				if (offset >= rtData.size())
				{
					return PA_ERROR(PeError::InvalidResourceSectionNameEntry);
				}
				auto sizeData = rtData.subspan(offset, sizeof(uint16_t));
				uint16_t size = 0;
				if (!TakeInto(sizeData, size))
				{
					return PA_ERROR(PeError::InvalidResourceSectionNameEntry);
				}
				std::wstring name;
				name.resize(size + 1);
				std::memcpy(name.data(), rtData.data() + offset + sizeof(uint16_t), size * sizeof(wchar_t));
				resource.Id = name;
			}
			else
			{
				resource.Id = nameEntry.Name & ~(1 << 31);
			}

			if (!nameEntry.GetBit())  // these should all be dirs
			{
				return PA_ERROR(PeError::InvalidResourceSectionNameEntry);
			}

			const uint32_t langOffset = nameEntry.GetOffset();
			if (langOffset >= rtData.size())
			{
				return PA_ERROR(PeError::InvalidResourceSectionLangEntry);
			}
			auto langData = rtData.subspan(langOffset, rtData.size() - langOffset);

			ImageResourceDirectory langRoot;
			if (!TakeInto(langData, langRoot))
			{
				return PA_ERROR(PeError::InvalidResourceSectionLangEntry);
			}

			if (langRoot.NumberOfNamedEntries != 0 || langRoot.NumberOfIdEntries != 1)
			{
				return PA_ERROR(PeError::InvalidResourceSectionLangEntry);
			}

			PA_TRY(langEntry, ImageResourceDirectoryEntry::FromData(langData));

			resource.LanguageId = langEntry.Name;

			if (langEntry.GetBit())
			{
				return PA_ERROR(PeError::InvalidResourceSectionLangEntry);
			}

			const uint32_t dataOffset = langEntry.GetOffset();
			if (dataOffset >= rtData.size())
			{
				return PA_ERROR(PeError::InvalidResourceSectionDataEntry);
			}
			auto dataData = rtData.subspan(dataOffset, sizeof(ImageResourceDataEntry));
			ImageResourceDataEntry dataEntry;
			if (!TakeInto(dataData, dataEntry))
			{
				return PA_ERROR(PeError::InvalidResourceSectionDataEntry);
			}

			if (dataEntry.OffsetToData - rva + dataEntry.Size > fullData.size())
			{
				return PA_ERROR(PeError::InvalidResourceSectionDataEntry);
			}

			resource.Data = fullData.subspan(dataEntry.OffsetToData - rva, dataEntry.Size);
			resourceTable.Resources.emplace_back(resource);
		}
	}

	return resourceTable;
}

Result<void, PeError> PeReader::Parse()
{
	std::span<const Byte> data = m_data;

	// PE Header
	PA_TRYA(m_peHeader, PeHeader::FromData(data));
	if (m_peHeader->DosHeader.CoffHeaderPointer >= m_data.size())
	{
		return PA_ERROR(PeError::InvalidCoffHeaderPtr);
	}

	// NT Header
	data = m_data.subspan(m_peHeader->DosHeader.CoffHeaderPointer, m_data.size() - m_peHeader->DosHeader.CoffHeaderPointer);
	PA_TRYA(m_ntHeader, NtHeader::FromData(data));

	// Section Headers
	const uint32_t shSize = SectionHeaderSize * m_ntHeader->ImageFileHeader.NumberOfSections;
	if (shSize > data.size())
	{
		return PA_ERROR(PeError::InvalidSectionHeaders);
	}
	auto shData = Take(data, shSize);
	m_sectionHeaders.reserve(m_ntHeader->ImageFileHeader.NumberOfSections);
	for (uint16_t i = 0; i < m_ntHeader->ImageFileHeader.NumberOfSections; i++)
	{
		PA_TRY(section, SectionHeader::FromData(Take(shData, SectionHeaderSize)));
		m_sectionHeaders.emplace_back(section);
	}

	uint32_t resDirAddress = 0;
	std::visit([&resDirAddress](auto&& arg)
	{
		using Header = std::decay_t<decltype(arg)>;
		if constexpr (std::is_same_v<Header, OptionalHeader32>)
		{
			resDirAddress = arg.Directories[static_cast<int>(ImageDirectoryEntryType::Resource)].VirtualAddress;
		}
		else if constexpr (std::is_same_v<Header, OptionalHeader64>)
		{
			resDirAddress = arg.Directories[static_cast<int>(ImageDirectoryEntryType::Resource)].VirtualAddress;
		}
		else
		{
			static_assert(always_false<Header>, "Unknown Optional Header variant");
		}
	}, m_ntHeader->OptionalHeader);

	for (const SectionHeader& h : m_sectionHeaders)
	{
		if (h.VirtualAddress <= resDirAddress && h.VirtualAddress + h.SizeOfRawData > resDirAddress)
		{
			const uint32_t rva = h.VirtualAddress - h.PtrRawData;
			const uint32_t resOffset = resDirAddress - rva;
			if (resOffset > m_data.size())
			{
				return PA_ERROR(PeError::InvalidSectionHeaders);
			}
			auto resData = m_data.subspan(resOffset, h.VirtualSize);
			PA_TRYA(m_table, ResourceTable::FromData(resData, m_data, rva));
		}
	}

	return {};
}
