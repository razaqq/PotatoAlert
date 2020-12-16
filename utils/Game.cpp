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
#include <tinyxml2.h>


using PotatoAlert::Game;
using PotatoAlert::folderStatus;
namespace fs = std::filesystem;

folderStatus Game::checkPath(const std::string& selectedPath)
{
	folderStatus status;
	status.gamePath = selectedPath;

	fs::path gamePath(selectedPath);
	if (selectedPath.empty() || !fs::exists(gamePath))
		return status;

	status.steamVersion = fs::exists(gamePath / "bin" / "clientrunner");
	Game::getResFolderPath(status);
	if (Game::readEngineConfig(status, "res"))
	{
    	if (Game::readPreferences(status))
    	{
    		std::string resModsFolder = fs::path(fs::path(status.resFolderPath) / "res_mods").string();
    		Game::readEngineConfig(status, resModsFolder.c_str());
    		Game::setReplaysFolder(status);
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
    	else
    	{
    		status.statusText = "Failed to read preferences";
    		return status;
    	}
	}
	else
	{
		status.statusText = "Failed to read engine config";
		return status;
	}
}

// finds the res folder path in the currently selected directory
bool Game::getResFolderPath(folderStatus& status)
{
    // get newest folder version inside /bin folder
    int folderVersion = -1;
    auto binPath = fs::path(status.gamePath) / "bin";
    if (fs::exists(binPath))
    {
        for (const auto& entry : fs::directory_iterator(binPath))
        {
            if (entry.is_directory())
            {
                try
                {
                    int v = std::stoi(entry.path().filename().string());
                    if (v > folderVersion)
                        folderVersion = v;
                }
                catch (std::invalid_argument& ia)
                {
                    // ignore folders that aren't a valid version
                }
            }
        }
    }

    if (folderVersion != -1)
    {
        status.folderVersion = std::to_string(folderVersion);
        status.resFolderPath = (fs::path(status.gamePath) / "bin" / status.folderVersion).string();
        return fs::exists(status.resFolderPath);
    }
    else
    {
		Logger::Error("Could not find a valid res folder!");
        return false;
    }
}

// reads the engine config and sets values
bool Game::readEngineConfig(folderStatus& status, const char* resFolder)
{
    fs::path engineConfig(status.resFolderPath / fs::path(resFolder) / "engine_config.xml");
	if (fs::exists(engineConfig))
	{
	    auto toLower = [](const char* text){
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
	else
	{
        Logger::Debug("No engine_config.xml in path: {}", resFolder);
		return false;
	}
}

// reads game version and region from preferences.xml
bool Game::readPreferences(folderStatus& status)
{
	// For some reason preferences.xml is not valid xml and so we have to parse it with regex instead of xml
    std::string preferencesPath;
	if (status.preferencesPathBase == "cwd")
        preferencesPath = (fs::path(status.gamePath) / "preferences.xml").string();
	else if (status.preferencesPathBase == "exe_path")
		preferencesPath = (fs::path(status.gamePath) / "bin" / status.folderVersion / "preferences.xml").string();

	if (fs::exists(fs::path(preferencesPath)))
	{
		std::ifstream ifs(preferencesPath);
		std::stringstream ss;
		ss << ifs.rdbuf();
		ifs.close();
		std::string pref = ss.str();

		static const std::regex versionRegex(R"regex(<clientVersion>([\s,0-9]*)<\/clientVersion>)regex");
		static const std::regex regionRegex(R"regex(<active_server>([\sA-Z]*)<\/active_server>)regex");
		std::smatch versionMatch;
		std::smatch regionMatch;

		if (std::regex_search(pref, versionMatch, versionRegex) && versionMatch.size() > 1)
		{
			status.gameVersion = versionMatch.str(1);
			std::replace(status.gameVersion.begin(), status.gameVersion.end(), ',', '.');
			status.gameVersion.erase(std::remove(status.gameVersion.begin(), status.gameVersion.end(), '\t'),
									 status.gameVersion.end());
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
	else
	{
		Logger::Error("Cannot find preferences.xml for reading in path: {}", preferencesPath);
		return false;
	}
}

// sets the replays folder
void Game::setReplaysFolder(folderStatus& status) {
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
