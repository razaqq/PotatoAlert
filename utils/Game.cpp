// Copyright 2020 <github.com/razaqq>

#include <string>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <regex>
#include <vector>
#include <algorithm>
#include "Logger.hpp"
#include "Game.hpp"
#include "Config.hpp"
#include <tinyxml2.h>
#include <winver.h>


using PotatoAlert::Game::folderStatus;
namespace fs = std::filesystem;

namespace PotatoAlert::Game {

// reads version number from a file
std::pair<bool, std::string> getFileVersion(const std::string& file)
{
	DWORD size = GetFileVersionInfoSize(file.c_str(), nullptr);
	if (size == 0)
		return {false, ""};

	char* versionInfo = new char[size];
	if (!GetFileVersionInfo(file.c_str(), 0, 255, versionInfo))
	{
		delete[] versionInfo;
		return {false, ""};
	}

	VS_FIXEDFILEINFO* out;
	UINT outSize = 0;
	if (!VerQueryValue(&versionInfo[0], "\\", (LPVOID*)&out, &outSize) && outSize > 0)
	{
		delete[] versionInfo;
		return {false, ""};
	}

	std::string version = fmt::format("{}.{}.{}.{}",
									  ( out->dwFileVersionMS >> 16 ) & 0xff,
									  ( out->dwFileVersionMS >>  0 ) & 0xff,
									  ( out->dwFileVersionLS >> 16 ) & 0xff,
									  ( out->dwFileVersionLS >>  0 ) & 0xff
									  );
	delete[] versionInfo;
	return {true, version};
}

// finds the res folder path in the currently selected directory
bool getResFolderPath(folderStatus& status)
{
	// get newest folder version inside /bin folder

	const fs::path binPath = fs::path(status.gamePath) / "bin";
	if (!fs::exists(binPath))
	{
		Logger::Error("Bin folder does not exist: {}", binPath.string());
		return false;
	}

	// try to find matching exe version to game version
	for (const auto& entry : fs::directory_iterator(binPath))
	{
		if (!entry.is_directory())
			continue;

		const fs::path exe = (entry.path() / "bin32" / "WorldOfWarships32.exe");

		auto&& [success, version] = getFileVersion(exe.string());
		if (!success)
			continue;

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

		try
		{
			const int v = std::stoi(entry.path().filename().string());
			if (v > folderVersion)
				folderVersion = v;
		}
		catch (const std::invalid_argument& ia)
		{
			// ignore folders that aren't a valid version
		}
	}

	if (folderVersion != -1)
	{
		status.folderVersion = std::to_string(folderVersion);
		status.resFolderPath = (fs::path(status.gamePath) / "bin" / status.folderVersion).string();
		return fs::exists(status.resFolderPath);
	}

	Logger::Error("Could not find a valid res folder!");
	return false;
}

// reads the engine config and sets values
bool readEngineConfig(folderStatus& status, const char* resFolder)
{
	fs::path engineConfig(status.resFolderPath / fs::path(resFolder) / "engine_config.xml");
	if (!fs::exists(engineConfig))
	{
		Logger::Debug("No engine_config.xml in path: {}", resFolder);
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
		Logger::Debug("Failed to open engine_config.xml for reading.");
		return false;
	}

	tinyxml2::XMLNode* root = doc.FirstChild();
	if (root == nullptr)
	{
		Logger::Debug("engine_config.xml seems to be empty.");
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
bool readPreferences(folderStatus& status, const std::string& basePath)
{
	// For some reason preferences.xml is not valid xml and so we have to parse it with regex instead of xml
	std::string preferencesPath = (fs::path(basePath) / "preferences.xml").string();
	/*
	if (status.preferencesPathBase == "cwd")
		preferencesPath = (fs::path(status.gamePath) / "preferences.xml").string();
	else if (status.preferencesPathBase == "exe_path")
		preferencesPath = (fs::path(status.gamePath) / "bin" / status.folderVersion / "preferences.xml").string();
	 */

	if (!fs::exists(fs::path(preferencesPath)))
	{
		Logger::Debug("Cannot find preferences.xml for reading in path: {}", preferencesPath);
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
		Logger::Error("Cannot find version string in preferences.xml.");
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
		Logger::Error("Cannot find region string in preferences.xml.");
		return false;
	}

	return true;
}

// sets the replays folder
void setReplaysFolder(folderStatus& status)
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

folderStatus checkPath(const std::string& selectedPath)
{
	folderStatus status{.gamePath = fs::path(selectedPath).make_preferred().string()};

	// check for replays folder override
	if (PotatoConfig().get<bool>("override_replays_folder"))
	{
		status.overrideReplaysPath = { PotatoConfig().get<std::string>("replays_folder") };
	}

	fs::path gamePath(status.gamePath);
	if (status.gamePath.empty() || !fs::exists(gamePath))
		return status;

	status.steamVersion = fs::exists(gamePath / "bin" / "clientrunner");

	readPreferences(status, status.gamePath);
	if (!getResFolderPath(status))
	{
		status.statusText = "Failed to get res folder path";
		return status;
	}

	if (!readEngineConfig(status, "res"))
	{
		status.statusText = "Failed to read engine config";
		return status;
	}

	std::string resModsFolder = fs::path(fs::path(status.resFolderPath) / "res_mods").string();
	readEngineConfig(status, resModsFolder.c_str());
	setReplaysFolder(status);
	bool found = true;
	for (auto& replaysPath : status.replaysPath)
	{
		if (!fs::exists(replaysPath))
			found = false;
	}
	status.statusText = found ? "Found" : "Replays folder doesn't exist";
	status.found = found;
	return status;
}

} // namespace Game
