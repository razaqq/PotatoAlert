#pragma once

#include "Core/Bytes.hpp"
#include "Core/Result.hpp"

#include <string>
#include <span>
#include <unordered_map>


namespace PotatoAlert::GameFileUnpack {

enum class MoFileError
{
	InvalidHeader,
	InvalidLength,
	UnsupportedRevision,
	UnsupportedEndianness,
	InvalidOffset,
};

static inline constexpr std::string_view ErrorMessage(MoFileError error)
{
	switch (error)
	{
		case MoFileError::InvalidHeader:
			return "Invalid header encountered";
		case MoFileError::InvalidLength:
			return "Invalid data length";
		case MoFileError::UnsupportedRevision:
			return "Unsupported revision encounterd";
		case MoFileError::UnsupportedEndianness:
			return "Unsupported Endianness encountered, data has to match system";
		case MoFileError::InvalidOffset:
			return "Out of bounds string offset encountered";
		default: 
			return "";
	}
}

typedef std::unordered_map<std::string_view, std::string_view> LocalizationTable;
Core::Result<LocalizationTable, MoFileError> ParseMachineObjectFile(std::span<const Core::Byte> data);

}  // namespace PotatoAlert::GameFileUnpack
