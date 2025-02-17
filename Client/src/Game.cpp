// Copyright 2020 <github.com/razaqq>

#include "Client/Config.hpp"
#include "Client/Game.hpp"

#include "Core/Defer.hpp"
#include "Core/Directory.hpp"
#include "Core/File.hpp"
#include "Core/FileMapping.hpp"
#include "Core/Format.hpp"
#include "Core/Log.hpp"
#include "Core/PeFileVersion.hpp"
#include "Core/String.hpp"
#include "Core/Version.hpp"
#include "Core/Xml.hpp"

#include <ctre.hpp>

#include <filesystem>
#include <ranges>
#include <string>
#include <vector>


using namespace PotatoAlert::Core;
namespace fs = std::filesystem;

namespace PotatoAlert::Client::Game {

namespace {

struct GameCategoryT : std::error_category
{
	const char* name() const noexcept override
	{
		return "GameError";
	}

	std::string message(int code) const override
	{
		switch (static_cast<GameError>(code))
		{
			using enum GameError;

			case DirectoryNotFound:
				return "Directory Not Found";
			case PreferencesXmlMissing:
				return "No preferences.xml";
			case PreferencesXmlFailedToMap:
				return "Failed to map preferences.xml";
			case PreferencesXmlMissingVersion:
				return "No version in preferences.xml";
			case PreferencesXmlMissingRegion:
				return "No region in preferences.xml";
			case PreferencesXmlInvalidRegion:
				return "Invalid region in preferences.xml";
			case BinPathMissing:
				return "Missing bin path";
			case BinPathFailedToReadPeVersion:
				return "Failed to read PE Version";
			case BinPathFailedToDetermine:
				return "Failed to determine bin path";
			case EngineConfigXmlMissing:
				return "Missing engine_config.xml";
			case EngineConfigXmlFailedLoading:
				return "Failed to read engine_config.xml";
			case EngineConfigXmlEmpty:
				return "Empty engine_config.xml";
			case EngineConfigXmlMissingReplays:
				return "engine_config.xml missing 'replays'";
			case EngineConfigXmlMissingDirPath:
				return "engine_config.xml missing 'dirPath'";
			case EngineConfigXmlMissingPathBase:
				return "engine_config.xml missing 'pathBase'";
			case EngineConfigXmlMissingVersioned:
				return "engine_config.xml missing 'versioned'";
			case EngineConfigXmlMissingPreferences:
				return "engine_config.xml missing 'preferences'";
			case EngineConfigXmlMissingPreferencesPathBase:
				return "engine_config.xml missing 'preferences/pathBase'";
			case EngineConfigXmlVersionedInvalid:
				return "engine_config.xml invalid 'versioned'";
		}

		return fmt::format("GameError{:08x}", static_cast<uint32_t>(code));
	}
};

const GameCategoryT g_gameCategory;

inline std::error_code MakeErrorCode(const GameError error)
{
	return { static_cast<int>(error), g_gameCategory };
}

struct PreferencesResult
{
	Version GameVersion;
	std::string Region;
};

Result<PreferencesResult> ReadPreferences(const fs::path& path)
{
	// For some reason preferences.xml is not valid xml, and so we have to parse it with regex instead of xml
	const fs::path preferencesPath = fs::path(path) / "preferences.xml";

	if (!PathExists(preferencesPath))
	{
		LOG_TRACE(STR("Cannot find preferences.xml for reading in path: {}"), preferencesPath);
		return PA_ERROR(MakeErrorCode(GameError::PreferencesXmlMissing));
	}

	const File file = File::Open(preferencesPath, File::Flags::Open | File::Flags::Read);
	if (!file)
	{
		return PA_ERROR(MakeErrorCode(GameError::PreferencesXmlFailedToMap));
	}
	uint64_t size = file.Size();
	FileMapping mapping = FileMapping::Open(file, FileMapping::Flags::Read, size);
	if (!mapping)
	{
		return PA_ERROR(MakeErrorCode(GameError::PreferencesXmlFailedToMap));
	}
	void* filePtr = mapping.Map(FileMapping::Flags::Read, 0, size);
	if (!filePtr)
	{
		return PA_ERROR(MakeErrorCode(GameError::PreferencesXmlFailedToMap));
	}
	PA_DEFER { mapping.Unmap(filePtr, size); };

	std::string_view preferences(static_cast<const char*>(filePtr), size);

	PreferencesResult result;
	if (auto [whole, version] = ctre::search<R"(<clientVersion>\s*([ ,0-9]*)\s*<\/clientVersion>)">(preferences); whole)
	{
		result.GameVersion = Version(version.to_view());
	}
	else
	{
		return PA_ERROR(MakeErrorCode(GameError::PreferencesXmlMissingVersion));
	}

	std::string server;
	if (auto [whole, region] = ctre::search<R"(<active_server>([a-z,A-Z,0-9,\s]*)<\/active_server>)">(preferences); whole)
	{
		result.Region = String::ToLower(String::Trim(region.to_view()));
	}
	else
	{
		return PA_ERROR(MakeErrorCode(GameError::PreferencesXmlMissingRegion));
	}

	if (result.Region.starts_with("&#"))
	{
		std::string_view r(result.Region);
		std::string out;
		out.reserve(2 * result.Region.size());
		while (true)
		{
			if (r.size() >= 2 && r[0] == '&' && result.Region[1] == '#')
			{
				const size_t j = r.find_first_not_of("0123456789", 2);
				if (j == std::string_view::npos)
				{
					PA_ERROR(MakeErrorCode(GameError::PreferencesXmlInvalidRegion));
				}

				if (r[j] != ';')
				{
					PA_ERROR(MakeErrorCode(GameError::PreferencesXmlInvalidRegion));
				}

				unsigned char c;
				if (!String::ParseNumber(r.substr(2, j - 2), c))
				{
					PA_ERROR(MakeErrorCode(GameError::PreferencesXmlInvalidRegion));
				}
				out.push_back(static_cast<char>(c));

				r = r.substr(j + 1);
			}
			else if (!r.empty())
			{
				out.push_back(r[0]);
				r = r.substr(1);
			}
			else
			{
				break;
			}
		}

		if (out == "\u041C\u0418\u0420 \u041A\u041E\u0420\u0410\u0411\u041B\u0415\u0419")
		{
			result.Region = "ru";
		}
	}
	else
	{
		String::ReplaceAll(result.Region, "wows ", "");
		String::ReplaceAll(result.Region, "cis", "ru");
		String::ReplaceAll(result.Region, "360", "china");
	}

	return result;
}

Result<fs::path> GetBinPath(const fs::path& gamePath, const Version gameVersion)
{
	// get newest folder version inside /bin folder

	const fs::path binPath = gamePath / "bin";
	PA_TRY(exists, PathExists(binPath));
	if (!exists)
	{
		return PA_ERROR(MakeErrorCode(GameError::BinPathMissing));
	}

	// try to find matching exe version to game version
	for (const fs::directory_entry& entry : fs::directory_iterator(binPath))
	{
		if (!entry.is_directory())
			continue;

		const fs::path exe = entry.path() / "bin64" / "WorldOfWarships64.exe";
		if (!fs::exists(exe))
		{
			continue;
		}

		const auto fileVersion = ReadFileVersion(exe);
		if (!fileVersion)
		{
			return PA_ERROR(MakeErrorCode(GameError::BinPathFailedToReadPeVersion));
		}

		if (*fileVersion == gameVersion)
		{
			std::string directoryVersion = entry.path().filename().string();

			return binPath / entry.path().filename().string();
		}
	}

	// fallback, take the biggest version number
	int32_t folderVersion = -1;
	for (const auto& entry : fs::directory_iterator(binPath))
	{
		if (!entry.is_directory())
		{
			continue;
		}

		std::string fileName = entry.path().filename().string();

		if (int32_t v = 0; String::ParseNumber<int32_t>(fileName, v))
		{
			if (v > folderVersion)
			{
				folderVersion = v;
			}
		}
	}

	if (folderVersion != -1)
	{
		return binPath / std::to_string(folderVersion);
	}

	return PA_ERROR(MakeErrorCode(GameError::BinPathFailedToDetermine));
}

struct EngineConfigResult
{
	bool ReplaysVersioned;
	std::string ReplaysPathBase;
	std::string ReplaysDirPath;
	std::string PreferencesPathBase;
};

Result<EngineConfigResult> ReadEngineConfig(const fs::path& file)
{
	if (!fs::exists(file))
	{
		return PA_ERROR(MakeErrorCode(GameError::EngineConfigXmlMissing));
	}

	tinyxml2::XMLDocument doc;
	XmlResult<void> res = LoadXml(doc, file);
	if (!res)
	{
		return PA_ERROR(MakeErrorCode(GameError::EngineConfigXmlFailedLoading));
	}

	tinyxml2::XMLNode* root = doc.FirstChildElement("engine_config.xml");
	if (root == nullptr)
	{
		return PA_ERROR(MakeErrorCode(GameError::EngineConfigXmlEmpty));
	}

	// get replays node
	tinyxml2::XMLElement* replays = root->FirstChildElement("replays");
	if (replays == nullptr)
	{
		return PA_ERROR(MakeErrorCode(GameError::EngineConfigXmlMissingReplays));
	}

	// get dir path
	tinyxml2::XMLElement* replaysDirPath = replays->FirstChildElement("dirPath");
	if (replaysDirPath == nullptr)
	{
		return PA_ERROR(MakeErrorCode(GameError::EngineConfigXmlMissingDirPath));
	}
	const std::string dirPath = String::ToLower(replaysDirPath->GetText());

	// get base path
	tinyxml2::XMLElement* replaysPathBase = replays->FirstChildElement("pathBase");
	if (replaysPathBase == nullptr)
	{
		return PA_ERROR(MakeErrorCode(GameError::EngineConfigXmlMissingPathBase));
	}
	const std::string pathBase = String::ToLower(replaysPathBase->GetText());

	// check for versioned replays
	tinyxml2::XMLElement* versionedReplays = replays->FirstChildElement("versioned");
	if (versionedReplays == nullptr)
	{
		return PA_ERROR(MakeErrorCode(GameError::EngineConfigXmlMissingVersioned));
	}

	bool versioned;
	if (!String::ParseBool(versionedReplays->GetText(), versioned))
	{
		return PA_ERROR(MakeErrorCode(GameError::EngineConfigXmlVersionedInvalid));
	}

	// get preferences node
	tinyxml2::XMLElement* preferences = root->FirstChildElement("preferences");
	if (preferences == nullptr)
	{
		return PA_ERROR(MakeErrorCode(GameError::EngineConfigXmlMissingPreferences));
	}

	// get preferences base path
	tinyxml2::XMLElement* preferencesPathBase = preferences->FirstChildElement("pathBase");
	if (preferencesPathBase == nullptr)
	{
		return PA_ERROR(MakeErrorCode(GameError::EngineConfigXmlMissingPreferencesPathBase));
	}
	const std::string prefPathBase = String::ToLower(preferencesPathBase->GetText());

	return EngineConfigResult
	{
		.ReplaysVersioned = versioned,
		.ReplaysPathBase = pathBase,
		.ReplaysDirPath = dirPath,
		.PreferencesPathBase = prefPathBase,
	};
}

}

Result<GameInfo> ReadGameInfo(const fs::path& path)
{
	PA_TRY(exists, Core::PathExists(path));
	if (!exists)
	{
		return PA_ERROR(MakeErrorCode(GameError::DirectoryNotFound));
	}

	PA_TRY(preferences, ReadPreferences(path));
	PA_TRY(binPath, GetBinPath(path, preferences.GameVersion));
	// engine config in res_mods takes precedence
	const fs::path resModsEngineCfg = binPath / "res_mods" / "engine_config.xml";
	PA_TRY(engineConfig, ReadEngineConfig(File::Exists(resModsEngineCfg) ? resModsEngineCfg : binPath / "res" / "engine_config.xml"));

	const fs::path idxPath = binPath / "idx";
	const fs::path pkgPath = path / "res_packages";

	std::vector<fs::path> replaysPaths;
	if (engineConfig.ReplaysPathBase == "cwd")
	{
		replaysPaths = { path / engineConfig.ReplaysDirPath };
	}
	else if (engineConfig.ReplaysPathBase == "exe_path")
	{
		replaysPaths = {
			binPath / "bin32" / engineConfig.ReplaysDirPath,
			binPath / "bin64" / engineConfig.ReplaysDirPath
		};
	}

	if (engineConfig.ReplaysVersioned)
	{
		for (fs::path& p : replaysPaths)
		{
			 p = p / preferences.GameVersion.ToString();
		}
	}

	return GameInfo
	{
		.GameVersion = preferences.GameVersion,
		.BinPath = binPath,
		.IdxPath = idxPath,
		.PkgPath = pkgPath,
		.VersionedReplays = engineConfig.ReplaysVersioned,
		.ReplaysPaths = std::move(replaysPaths),
		.Region = preferences.Region,
	};
}

}  // namespace PotatoAlert::Client::Game
