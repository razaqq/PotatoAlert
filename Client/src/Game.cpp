// Copyright 2020 <github.com/razaqq>

#include "Client/Config.hpp"
#include "Client/Game.hpp"

#include "Core/File.hpp"
#include "Core/Log.hpp"
#include "Core/String.hpp"
#include "Core/Version.hpp"

#include <tinyxml2.h>

#include <filesystem>
#include <regex>
#include <string>
#include <vector>


using namespace PotatoAlert::Core;
using PotatoAlert::Client::Game::DirectoryStatus;
namespace fs = std::filesystem;

namespace PotatoAlert::Client::Game {

bool GetBinPath(DirectoryStatus& status)
{
	// get newest folder version inside /bin folder

	const fs::path binPath = fs::path(status.gamePath) / "bin";
	if (!fs::exists(binPath))
	{
		LOG_ERROR("Bin folder does not exist: {}", binPath.string());
		return false;
	}

	// try to find matching exe version to game version
	for (const auto& entry : fs::directory_iterator(binPath))
	{
		if (!entry.is_directory())
			continue;

		const fs::path exe = (entry.path() / "bin32" / "WorldOfWarships32.exe");
		if (!fs::exists(exe))
		{
			continue;
		}

		Version version;
		if (!File::GetVersion(exe.string(), version) || !version)
		{
			LOG_ERROR("Failed to read version info from file: {} - {}", exe.string(), File::LastError());
			continue;
		}

		if (version == status.gameVersion)
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
bool ReadEngineConfig(DirectoryStatus& status, const char* resFolder)
{
	fs::path engineConfig(status.binPath / fs::path(resFolder) / "engine_config.xml");
	if (!fs::exists(engineConfig))
	{
		LOG_TRACE("No engine_config.xml in path: {}", resFolder);
		return false;
	}

	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError err = doc.LoadFile(engineConfig.string().c_str());
	if (err != tinyxml2::XML_SUCCESS)
	{
		LOG_TRACE("Failed to open engine_config.xml for reading.");
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
bool ReadPreferences(DirectoryStatus& status, const std::string& basePath)
{
	// For some reason preferences.xml is not valid xml and so we have to parse it with regex instead of xml
	std::string preferencesPath = (fs::path(basePath) / "preferences.xml").string();

	if (!fs::exists(fs::path(preferencesPath)))
	{
		LOG_TRACE("Cannot find preferences.xml for reading in path: {}", preferencesPath);
		return false;
	}

	std::string pref;
	if (File file = File::Open(preferencesPath, File::Flags::Open | File::Flags::Read); file)
	{
		if (!file.ReadString(pref))
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
	static const std::regex regionRegex(R"regex(<active_server>([\sA-Z]*)<\/active_server>)regex");
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
		status.region = String::ToLower(String::Trim(regionMatch.str(1)));
		status.region = std::regex_replace(status.region, std::regex("wows "), "");  // remove 'WOWS '
		status.region = std::regex_replace(status.region, std::regex("cis"), "ru");  // cis server to ru
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
		status.replaysPath = {(fs::path(status.gamePath) / status.replaysDirPath).string()};
	}
	else if (status.replaysPathBase == "exe_path")
	{
		status.replaysPath = {
				(status.binPath / "bin32" / status.replaysDirPath).string(),
				(status.binPath / "bin64" / status.replaysDirPath).string()
		};
	}
	if (status.versionedReplays)
	{
		std::vector<std::string> newReplaysPath;
		for (auto& path : status.replaysPath)
		{
			newReplaysPath.push_back((fs::path(path) / status.gameVersion.ToString()).string());
		}
		status.replaysPath = newReplaysPath;
	}
}

bool CheckPath(const std::string& selectedPath, DirectoryStatus& status)
{
	const fs::path gamePath = fs::path(selectedPath).make_preferred();
	status.gamePath = gamePath.string();

	// check that the game path exists
	std::error_code ec;
	bool exists = fs::exists(gamePath, ec);
	if (ec)
	{
		LOG_ERROR("Failed to check if game path exists: {}", ec.message());
		return false;
	}
	if (gamePath.empty() || !exists)
		return false;

	ReadPreferences(status, status.gamePath);
	if (!GetBinPath(status))
	{
		status.statusText = "Failed to get bin path";
		return false;
	}

	status.idxPath = status.binPath / "idx";
	status.pkgPath = gamePath / "res_packages";

	if (!ReadEngineConfig(status, "res"))
	{
		status.statusText = "Failed to read engine config";
		return false;
	}

	const std::string resModsFolder = fs::path(fs::path(status.binPath) / "res_mods").string();
	ReadEngineConfig(status, resModsFolder.c_str());
	SetReplaysFolder(status);


	// get rid of all replays folders that don't exist
	status.replaysPath.erase(std::remove_if(status.replaysPath.begin(), status.replaysPath.end(), [](const std::string& p)
	{
		if (!fs::exists(p))
		{
			LOG_TRACE("Removing replays folder {}, because it doesn't exist.", p);
			return true;
		}
		return false;
	}), status.replaysPath.end());

	status.found = !status.replaysPath.empty();
	// TODO: localize
	status.statusText = status.found ? "Found" : "Replays folder doesn't exist";

	return true;
}

}  // namespace PotatoAlert::Client::Game
