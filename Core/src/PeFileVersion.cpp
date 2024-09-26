// Copyright 2024 <github.com/razaqq>

#include "Core/File.hpp"
#include "Core/FileMapping.hpp"
#include "Core/PeFileVersion.hpp"
#include "Core/PeReader.hpp"
#include "Core/Result.hpp"
#include "Core/TypeTraits.hpp"
#include "Core/Version.hpp"

#include <string>
#include <ranges>


using PotatoAlert::Core::FileVersionReadError;
using PotatoAlert::Core::FileVersionReadErrorMisc;
using PotatoAlert::Core::Result;
using PotatoAlert::Core::Version;

std::string_view PotatoAlert::Core::GetErrorMessage(FileVersionReadError error)
{
	return std::visit([](auto&& e) -> std::string_view
	{
		using E = std::decay_t<decltype(e)>;
		if constexpr (std::is_same_v<E, PeError>)
		{
			return GetErrorMessage(e);
		}
		else if constexpr (std::is_same_v<E, FileVersionReadErrorMisc>)
		{
			switch (e)
			{
				case FileVersionReadErrorMisc::FileOpenError:
					return "Failed to open file";
				case FileVersionReadErrorMisc::FileMapError:
					return "Failed to map file";
				case FileVersionReadErrorMisc::MissingResourceTable:
					return "Missing resource table";
				case FileVersionReadErrorMisc::MissingResourceVersionEntry:
					return "Resource table has no version entry";
			}
			return "Unknown Error";
		}
		else
		{
			static_assert(always_false<E>, "Unknown FileVersionReadError variant");
			return "";
		}
	}, error);
}

Result<Version, FileVersionReadError> PotatoAlert::Core::ReadFileVersion(const std::filesystem::path& p)
{
	if (const File file = File::Open(p, File::Flags::Open | File::Flags::Read))
	{
		const uint64_t fileSize = file.Size();
		if (FileMapping fileMapping = FileMapping::Open(file, FileMapping::Flags::Read, fileSize))
		{
			if (void const* filePtr = fileMapping.Map(FileMapping::Flags::Read, 0, fileSize))
			{
				PeReader peReader(std::span(static_cast<const Byte*>(filePtr), fileSize));
				PA_TRYV(peReader.Parse());

				const std::optional<ResourceTable> resTable = peReader.GetResourceTable();
				if (!resTable)
				{
					return PA_ERROR(FileVersionReadErrorMisc::MissingResourceTable);
				}

				const auto it = std::ranges::find_if(resTable->Resources, [](const Resource& resource)
				{
					return resource.Type == ResourceType::Version;
				});

				if (it == resTable->Resources.end())
				{
					return PA_ERROR(FileVersionReadErrorMisc::MissingResourceVersionEntry);
				}

				PA_TRY(versionInfo, VsVersionInfo::FromData(it->Data));

				Version version(
						versionInfo.Value.FileVersionMS >> 16 & 0xFF,
						versionInfo.Value.FileVersionMS >> 0 & 0xFF,
						versionInfo.Value.FileVersionLS >> 16 & 0xFF,
						versionInfo.Value.FileVersionLS >> 0 & 0xFF);
				return version;
			}
			return PA_ERROR(FileVersionReadErrorMisc::FileMapError);
		}
		return PA_ERROR(FileVersionReadErrorMisc::FileMapError);
	}
	return PA_ERROR(FileVersionReadErrorMisc::FileOpenError);
}
