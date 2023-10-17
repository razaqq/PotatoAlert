#include "GameFileUnpack/MoFile.hpp"

#include "Core/Bytes.hpp"
#include "Core/Result.hpp"

#include <bit>
#include <cstdint>
#include <span>
#include <string>
#include <unordered_map>


using PotatoAlert::Core::Byte;
using PotatoAlert::Core::Result;
using PotatoAlert::Core::TakeInto;
using PotatoAlert::GameFileUnpack::LocalizationTable;
using PotatoAlert::GameFileUnpack::MoFileError;

namespace {

struct Header
{
	uint32_t magic;
	int32_t revision;
	int32_t stringCount;
	int32_t originalOffset;
	int32_t translationOffset;
	int32_t hashtableSize;
	int32_t hashtableOffset;
};
static_assert(sizeof(Header) == 28);

struct StringInfo
{
	int32_t length;
	int32_t offset;
};
static_assert(sizeof(StringInfo) == 8);

}

Result<LocalizationTable, MoFileError> PotatoAlert::GameFileUnpack::ParseMachineObjectFile(std::span<const Byte> data)
{
	const std::span<const Byte> originalData = data;
	std::unordered_map<std::string_view, std::string_view> table;

	if (data.size() < sizeof(Header))
		return PA_ERROR(MoFileError::InvalidLength);

	Header header{};
	if (!TakeInto(data, header))
		return PA_ERROR(MoFileError::InvalidLength);

	if (header.magic == 0x950412DE)
	{
		if constexpr (std::endian::native != std::endian::little)
			return PA_ERROR(MoFileError::UnsupportedEndianness);
	}
	else if (header.magic == 0xDE120495)
	{
		if constexpr (std::endian::native != std::endian::big)
			return PA_ERROR(MoFileError::UnsupportedEndianness);
	}
	else
		return PA_ERROR(MoFileError::InvalidHeader);

	if (header.revision != 0)
		return PA_ERROR(MoFileError::UnsupportedRevision);

	// seek to offset
	data = originalData.subspan(header.originalOffset);

	// read original string infos
	std::vector<StringInfo> originals;
	originals.reserve(header.stringCount);
	for (int32_t i = 0; i < header.stringCount; i++)
	{
		StringInfo original{};
		if (!TakeInto(data, original))
			return PA_ERROR(MoFileError::InvalidLength);

		if (original.offset + original.length + 1 > originalData.size())
			return PA_ERROR(MoFileError::InvalidOffset);

		originals.emplace_back(original);
	}

	// seek to offset
	data = originalData.subspan(header.translationOffset);

	// read translations, insert
	for (int32_t i = 0; i < header.stringCount; i++)
	{
		const StringInfo& original = originals[i];

		StringInfo localization{};
		if (!TakeInto(data, localization))
			return PA_ERROR(MoFileError::InvalidLength);

		if (localization.offset + localization.length + 1 > originalData.size())
			return PA_ERROR(MoFileError::InvalidOffset);

		table.emplace(
				std::string_view(reinterpret_cast<const char*>(originalData.data() + original.offset), original.length + 1),
				std::string_view(reinterpret_cast<const char*>(originalData.data() + localization.offset), localization.length + 1)
		);
	}

	return table;
}
