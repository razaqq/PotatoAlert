// Copyright 2020 <github.com/razaqq>

#include "Client/Config.hpp"
#include "Client/Game.hpp"

#include "Core/File.hpp"
#include "Core/Log.hpp"
#include "Core/PeFileVersion.hpp"
#include "Core/String.hpp"
#include "Core/Version.hpp"
#include "Core/Xml.hpp"

#include <filesystem>
#include <ranges>
#include <regex>
#include <string>
#include <vector>


using namespace PotatoAlert::Core;
namespace fs = std::filesystem;

namespace PotatoAlert::Client::Game {

bool GetBinPath(DirectoryStatus& status)
{
	// get newest folder version inside /bin folder

	const fs::path binPath = status.gamePath / "bin";
	if (!fs::exists(binPath))
	{
		LOG_ERROR(STR("Bin folder does not exist: {}"), binPath);
		return false;
	}

	// try to find matching exe version to game version
	for (const auto& entry : fs::directory_iterator(binPath))
	{
		if (!entry.is_directory())
			continue;

		const fs::path exe = entry.path() / "bin32" / "WorldOfWarships32.exe";
		if (!fs::exists(exe))
		{
			continue;
		}

		const auto res = ReadFileVersion(exe);
		if (!res)
		{
			LOG_ERROR(STR("Failed to read file version from '{}': {}"), exe, GetErrorMessage(res.error()));
			continue;
		}

		if (*res == status.gameVersion)
		{
			status.directoryVersion = entry.path().filename().string();
			status.binPath = fs::path(status.gamePath) / "bin" / status.directoryVersion;
			return fs::exists(status.binPath);
		}
	}

	// fallback, take biggest version number
	int folderVersion = -1;
	for (const auto& entry : fs::directory_iterator(binPath))
	{
		if (!entry.is_directory())
			continue;

		std::string fileName = entry.path().filename().string();

		if (int v = 0; String::ParseNumber<int>(fileName, v))
			if (v > folderVersion)
				folderVersion = v;
	}

	if (folderVersion != -1)
	{
		status.directoryVersion = std::to_string(folderVersion);
		status.binPath = fs::path(status.gamePath) / "bin" / status.directoryVersion;
		return fs::exists(status.binPath);
	}

	LOG_ERROR("Could not find a valid res folder!");
	return false;
}

// reads the engine config and sets values
bool ReadEngineConfig(DirectoryStatus& status, const fs::path& resFolder)
{
	fs::path engineConfig(status.binPath / resFolder / "engine_config.xml");
	if (!fs::exists(engineConfig))
	{
		LOG_TRACE(STR("No engine_config.xml in path: {}"), resFolder);
		return false;
	}

	tinyxml2::XMLDocument doc;
	XmlResult<void> res = LoadXml(doc, engineConfig);
	if (!res)
	{
		LOG_TRACE("Failed to load engine_config.xml: {}", res.error());
		return false;
	}

	tinyxml2::XMLNode* root = doc.FirstChildElement("engine_config.xml");
	if (root == nullptr)
	{
		LOG_TRACE("engine_config.xml seems to be empty.");
		return false;
	}

	// get replays node
	tinyxml2::XMLElement* replays = root->FirstChildElement("replays");
	if (replays == nullptr)
	{
		LOG_ERROR("Key 'replays' is missing in engine_config.xml");
		return false;
	}

	// get dir path
	tinyxml2::XMLElement* replaysDirPath = replays->FirstChildElement("dirPath");
	if (replaysDirPath == nullptr)
	{
		LOG_ERROR("Key 'dirPath' is missing in engine_config.xml");
		return false;
	}
	status.replaysDirPath = String::ToLower(replaysDirPath->GetText());

	// get base path
	tinyxml2::XMLElement* replaysPathBase = replays->FirstChildElement("pathBase");
	if (replaysPathBase == nullptr)
	{
		LOG_ERROR("Key 'pathBase' is missing in engine_config.xml");
		return false;
	}
	status.replaysPathBase = String::ToLower(replaysPathBase->GetText());

	// check for versioned replays
	tinyxml2::XMLElement* versionedReplays = replays->FirstChildElement("versioned");
	if (versionedReplays == nullptr)
	{
		LOG_ERROR("Key 'versioned' is missing in engine_config.xml");
		return false;
	}

	bool versioned;
	if (!String::ParseBool(versionedReplays->GetText(), versioned))
	{
		LOG_ERROR("Cannot parse versionedReplays string to bool: '{}'", versionedReplays->GetText());
	}
	else
	{
		status.versionedReplays = versioned;
	}

	// get preferences node
	tinyxml2::XMLElement* preferences = root->FirstChildElement("preferences");
	if (preferences == nullptr)
	{
		LOG_ERROR("Key 'preferences' is missing in engine_config.xml");
		return false;
	}

	// get preferences base path
	tinyxml2::XMLElement* preferencesPathBase = replays->FirstChildElement("pathBase");
	if (preferencesPathBase == nullptr)
	{
		LOG_ERROR("Key 'pathBase' is missing in engine_config.xml");
		return false;
	}
	status.preferencesPathBase = String::ToLower(preferencesPathBase->GetText());

	return true;
}

// reads game version and region from preferences.xml
bool ReadPreferences(DirectoryStatus& status, const fs::path& basePath)
{
	// For some reason preferences.xml is not valid xml and so we have to parse it with regex instead of xml
	const fs::path preferencesPath = fs::path(basePath) / "preferences.xml";

	if (!fs::exists(preferencesPath))
	{
		LOG_TRACE(STR("Cannot find preferences.xml for reading in path: {}"), preferencesPath);
		return false;
	}

	std::string pref;
	if (const File file = File::Open(preferencesPath, File::Flags::Open | File::Flags::Read); file)
	{
		if (!file.ReadAllString(pref))
		{
			LOG_ERROR("Failed to read preferences.xml: {}", File::LastError());
			return false;
		}
	}
	else
	{
		LOG_ERROR("Failed to open preferences.xml for reading: {}", File::LastError());
		return false;
	}

	static const std::regex versionRegex(R"regex(<clientVersion>([\s,0-9]*)<\/clientVersion>)regex");
	static const std::regex regionRegex(R"regex(<active_server>(.*)<\/active_server>)regex");
	std::smatch versionMatch;
	std::smatch regionMatch;

	if (std::regex_search(pref, versionMatch, versionRegex) && versionMatch.size() > 1)
	{
		status.gameVersion = Version(versionMatch.str(1));
		// std::replace(status.gameVersion.begin(), status.gameVersion.end(), ',', '.');
	}
	else
	{
		LOG_ERROR("Cannot find version string in preferences.xml.");
		return false;
	}

	if (std::regex_search(pref, regionMatch, regionRegex) && regionMatch.size() > 1)
	{
		std::string region = String::ToLower(String::Trim(regionMatch.str(1)));
		if (region.starts_with("&#"))
		{
			std::string_view r(region);
			std::string out;
			out.reserve(2 * region.size());
			while (true)
			{
				if (r.size() >= 2 && r[0] == '&' && region[1] == '#')
				{
					const size_t j = r.find_first_not_of("0123456789", 2);
					if (j == std::string_view::npos)
					{
						return false;
					}

					if (r[j] != ';')
					{
						return false;
					}

					unsigned char c;
					if (!String::ParseNumber(r.substr(2, j - 2), c))
					{
						return false;
					}
					out.push_back(static_cast<char>(c));

					r = r.substr(j+1);
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
				status.region = "ru";
			}
		}
		else
		{
			region = std::regex_replace(region, std::regex("wows "), "");  // remove 'WOWS '
			region = std::regex_replace(region, std::regex("cis"), "ru");  // cis server to ru
			region = std::regex_replace(region, std::regex("360"), "china");  // 360 to china
			status.region = region;
		}
	}
	else
	{
		LOG_ERROR("Cannot find region string in preferences.xml.");
		return false;
	}

	return true;
}

// sets the replays folder
void SetReplaysFolder(DirectoryStatus& status)
{
	if (status.replaysPathBase == "cwd")
	{
		status.replaysPath = { fs::path(status.gamePath) / status.replaysDirPath };
	}
	else if (status.replaysPathBase == "exe_path")
	{
		status.replaysPath = {
			status.binPath / "bin32" / status.replaysDirPath,
			status.binPath / "bin64" / status.replaysDirPath
		};
	}
	if (status.versionedReplays)
	{
		std::vector<fs::path> newReplaysPath;
		newReplaysPath.reserve(status.replaysPath.size());
		for (auto& path : status.replaysPath)
		{
			newReplaysPath.emplace_back(path / status.gameVersion.ToString());
		}
		status.replaysPath = newReplaysPath;
	}
}

bool CheckPath(const fs::path& selectedPath, DirectoryStatus& status)
{
	status.gamePath = selectedPath;
	
	// check that the game path exists
	std::error_code ec;
	bool exists = fs::exists(status.gamePath, ec);
	if (ec)
	{
		LOG_ERROR("Failed to check if game path exists: {}", ec);
		return false;
	}
	if (status.gamePath.empty() || !exists)
		return false;

	ReadPreferences(status, status.gamePath);
	if (!GetBinPath(status))
	{
		status.statusText = "Failed to get bin path";
		return false;
	}

	status.idxPath = status.binPath / "idx";
	status.pkgPath = status.gamePath / "res_packages";

	if (!ReadEngineConfig(status, fs::path("res")))
	{
		status.statusText = "Failed to read engine config";
		return false;
	}

	ReadEngineConfig(status, status.binPath / "res_mods");
	SetReplaysFolder(status);

	// get rid of all replays folders that don't exist
	status.replaysPath.erase(std::ranges::remove_if(status.replaysPath, [](const fs::path& p)
	{
		if (!fs::exists(p))
		{
			LOG_TRACE(STR("Removing replays folder {}, because it doesn't exist."), p);
			return true;
		}
		return false;
	}).begin(), status.replaysPath.end());

	status.found = !status.replaysPath.empty();
	// TODO: localize
	status.statusText = status.found ? "Found" : "Replays folder doesn't exist";

	return true;
}

}  // namespace PotatoAlert::Client::Game
