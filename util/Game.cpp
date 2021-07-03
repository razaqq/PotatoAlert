// Copyright 2020 <github.com/razaqq>

#include "Game.hpp"

#include "Config.hpp"
#include "File.hpp"
#include "Log.hpp"

#include <tinyxml2.h>

#include <algorithm>
#include <charconv>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>


using PotatoAlert::Game::FolderStatus;
namespace fs = std::filesystem;

namespace PotatoAlert::Game {

static bool AsInteger(const std::string& src, int& value)
{
	auto [ptr, ec] = std::from_chars(src.data(), src.data()+src.size(), value);
	return ec == std::errc();
}

// finds the res folder path in the currently selected directory
bool GetResFolderPath(FolderStatus& status)
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

		std::string version;
		if (!File::GetVersion(exe.string(), version))
		{
			LOG_ERROR("Failed to read version info from file: {} - {}", exe.string(), File::LastError());
			continue;
		}

		if (version == status.gameVersion)
		{
			status.folderVersion = entry.path().filename().string();
			status.resFolderPath = (fs::path(status.gamePath) / "bin" / status.folderVersion).string();
			return fs::exists(status.resFolderPath);
		}
	}

	// fallback, take biggest version number
	int folderVersion = -1;
	for (const auto& entry : fs::directory_iterator(binPath))
	{
		if (!entry.is_directory())
			continue;

		std::string fileName = entry.path().filename().string();
		int v = 0;

		if (AsInteger(fileName, v))
			if (v > folderVersion)
				folderVersion = v;
	}

	if (folderVersion != -1)
	{
		status.folderVersion = std::to_string(folderVersion);
		status.resFolderPath = (fs::path(status.gamePath) / "bin" / status.folderVersion).string();
		return fs::exists(status.resFolderPath);
	}

	LOG_ERROR("Could not find a valid res folder!");
	return false;
}

// reads the engine config and sets values
bool ReadEngineConfig(FolderStatus& status, const char* resFolder)
{
	fs::path engineConfig(status.resFolderPath / fs::path(resFolder) / "engine_config.xml");
	if (!fs::exists(engineConfig))
	{
		LOG_TRACE("No engine_config.xml in path: {}", resFolder);
		return false;
	}

	auto toLower = [](const char* text)
	{
		auto textStr = std::string(text);
		std::transform(textStr.begin(), textStr.end(), textStr.begin(), ::tolower);
		return textStr;
	};

	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError err = doc.LoadFile(engineConfig.string().c_str());
	if (err != tinyxml2::XML_SUCCESS)
	{
		LOG_TRACE("Failed to open engine_config.xml for reading.");
		return false;
	}

	tinyxml2::XMLNode* root = doc.FirstChild();
	if (root == nullptr)
	{
		LOG_TRACE("engine_config.xml seems to be empty.");
		return false;
	}

	// get replays node
	tinyxml2::XMLElement* replays = root->FirstChildElement("replays");
	if (replays == nullptr)
		return false;

	// get dir path
	tinyxml2::XMLElement* replaysDirPath = replays->FirstChildElement("dirPath");
	if (replaysDirPath == nullptr)
		return false;
	status.replaysDirPath = toLower(replaysDirPath->GetText());

	// get base path
	tinyxml2::XMLElement* replaysPathBase = replays->FirstChildElement("pathBase");
	if (replaysPathBase == nullptr)
		return false;
	status.replaysPathBase = toLower(replaysPathBase->GetText());

	// check for versioned replays
	tinyxml2::XMLElement* versionedReplays = replays->FirstChildElement("versioned");
	if (versionedReplays == nullptr)
		return false;
	std::istringstream(toLower(versionedReplays->GetText())) >> std::boolalpha >> status.versionedReplays;

	// get preferences node
	tinyxml2::XMLElement* preferences = root->FirstChildElement("preferences");
	if (preferences == nullptr)
		return false;

	// get preferences base path
	tinyxml2::XMLElement* preferencesPathBase = replays->FirstChildElement("pathBase");
	if (preferencesPathBase == nullptr)
		return false;
	status.preferencesPathBase = toLower(preferencesPathBase->GetText());

	return true;
}

// reads game version and region from preferences.xml
bool ReadPreferences(FolderStatus& status, const std::string& basePath)
{
	// For some reason preferences.xml is not valid xml and so we have to parse it with regex instead of xml
	std::string preferencesPath = (fs::path(basePath) / "preferences.xml").string();

	if (!fs::exists(fs::path(preferencesPath)))
	{
		LOG_TRACE("Cannot find preferences.xml for reading in path: {}", preferencesPath);
		return false;
	}

	std::ifstream ifs(preferencesPath);
	std::stringstream ss;
	ss << ifs.rdbuf();
	ifs.close();
	const std::string pref = ss.str();

	static const std::regex versionRegex(R"regex(<clientVersion>([\s,0-9]*)<\/clientVersion>)regex");
	static const std::regex regionRegex(R"regex(<active_server>([\sA-Z]*)<\/active_server>)regex");
	std::smatch versionMatch;
	std::smatch regionMatch;

	if (std::regex_search(pref, versionMatch, versionRegex) && versionMatch.size() > 1)
	{
		status.gameVersion = versionMatch.str(1);
		std::replace(status.gameVersion.begin(), status.gameVersion.end(), ',', '.');
		status.gameVersion.erase(
				std::remove(status.gameVersion.begin(), status.gameVersion.end(), '\t'), status.gameVersion.end()
		);
	}
	else
	{
		LOG_ERROR("Cannot find version string in preferences.xml.");
		return false;
	}

	if (std::regex_search(pref, regionMatch, regionRegex) && regionMatch.size() > 1)
	{
		status.region = regionMatch.str(1);
		std::transform(status.region.begin(), status.region.end(), status.region.begin(), ::tolower);  // to lower
		status.region = std::regex_replace(status.region, std::regex("wows "), "");  // remove 'WOWS '
		status.region = std::regex_replace(status.region, std::regex("cis"), "ru");  // cis server to ru
		status.region.erase(std::remove(status.region.begin(), status.region.end(), '\t'), status.region.end());  // remove tabs
	}
	else
	{
		LOG_ERROR("Cannot find region string in preferences.xml.");
		return false;
	}

	return true;
}

// sets the replays folder
void SetReplaysFolder(FolderStatus& status)
{
	if (status.replaysPathBase == "cwd")
	{
		status.replaysPath = {(fs::path(status.gamePath) / status.replaysDirPath).string()};
	}
	else if (status.replaysPathBase == "exe_path")
	{
		status.replaysPath = {
				(fs::path(status.gamePath) / "bin" / status.folderVersion / "bin32" / status.replaysDirPath).string(),
				(fs::path(status.gamePath) / "bin" / status.folderVersion / "bin64" / status.replaysDirPath).string()
		};
	}
	if (status.versionedReplays)
	{
		std::vector<std::string> newReplaysPath;
		for (auto& path : status.replaysPath)
		{
			newReplaysPath.push_back((fs::path(path) / status.gameVersion).string());
		}
		status.replaysPath = newReplaysPath;
	}
}

bool CheckPath(const std::string& selectedPath, FolderStatus& status)
{
	fs::path gamePath = fs::path(selectedPath).make_preferred();
	status.gamePath = gamePath.string();

	// check for replays folder override
	if (PotatoConfig().Get<bool>("override_replays_folder"))
	{
		status.overrideReplaysPath = PotatoConfig().Get<std::string>("replays_folder");
	}

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
	if (!GetResFolderPath(status))
	{
		status.statusText = "Failed to get res folder path";
		return false;
	}

	if (!ReadEngineConfig(status, "res"))
	{
		status.statusText = "Failed to read engine config";
		return false;
	}

	std::string resModsFolder = fs::path(fs::path(status.resFolderPath) / "res_mods").string();
	ReadEngineConfig(status, resModsFolder.c_str());
	SetReplaysFolder(status);


	// get rid of all replays folders that dont exist
	status.replaysPath.erase(std::remove_if(status.replaysPath.begin(), status.replaysPath.end(), [](const std::string& p)
	{
		return !fs::exists(p);
	}), status.replaysPath.end());

	status.found = !status.replaysPath.empty();
	// TODO: localize
	status.statusText = status.found ? "Found" : "Replays folder doesn't exist";

	return true;
}

}  // namespace PotatoAlert::Game
