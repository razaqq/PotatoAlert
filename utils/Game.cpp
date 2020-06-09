// Copyright 2020 <github.com/razaqq>

#include <string>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <regex>
#include <algorithm>
#include "Logger.h"
#include "Game.h"
#include <tinyxml2.h>


using PotatoAlert::Game;
using PotatoAlert::folderStatus;
namespace fs = std::filesystem;

folderStatus Game::checkPath(const std::string& selectedPath, Logger* logger)
{
	folderStatus status;
	status.gamePath = selectedPath;

	fs::path gamePath(selectedPath);
	if (selectedPath != "" && fs::exists(gamePath))
	{
		status.steamVersion = fs::exists(gamePath / "bin" / "clientrunner");
		Game::getResFolderPath(status, logger);
		if (Game::readEngineConfig(status, logger))
		{

		}
	}
	return status;
}

void Game::getResFolderPath(folderStatus& status, Logger* logger)
{
	if (!status.steamVersion)
	{
		status.resPath = (fs::path(status.gamePath) / "res").string();
	}
	else
	{

	}
}

bool Game::readEngineConfig(folderStatus& status, Logger* logger)
{
	fs::path resPath(status.resPath);
	if (fs::exists(resPath / "engine_config.xml"))
	{
		std::string filePath = (resPath / "engine_config.xml").string();
		tinyxml2::XMLDocument doc;
		tinyxml2::XMLError err = doc.LoadFile(filePath.c_str());
		if (err != tinyxml2::XML_SUCCESS)
		{
			logger->Debug("Failed to open engine_config.xml for reading.");
			return false;
		}

		tinyxml2::XMLNode* root = doc.FirstChild();
		if (root == nullptr)
		{
			logger->Debug("engine_config.xml seems to be empty.");
			return false;
		}

		/*
		tinyxml2::XMLElement* preferences = root->FirstChildElement("preferences");
		if (preferences == nullptr)
			return;
		tinyxml2::XMLElement* pathBase = root->FirstChildElement("pathBase");
		if (pathBase == nullptr)
			return;
		pathBase->GetText();
		*/

		tinyxml2::XMLElement* replays = root->FirstChildElement("replays");
		if (replays == nullptr)
			return false;

		tinyxml2::XMLElement* replaysDirPath = replays->FirstChildElement("dirPath");
		if (replaysDirPath == nullptr)
			return false;
		status.replaysDirPath = replaysDirPath->GetText();

		tinyxml2::XMLElement* replaysPathBase = replays->FirstChildElement("pathBase");
		if (replaysPathBase == nullptr)
			return false;
		status.replaysPathBase = replaysPathBase->GetText();

		err = replays->QueryBoolAttribute("versioned", &status.versionedReplays);
		if (err != tinyxml2::XML_SUCCESS)
			return false;

		return true;
	}
	else
	{
		logger->Error("engine_config.xml does not exist in path: ");
		logger->Error(status.resPath.c_str());
		return false;
	}
}

// reads game version from preferences.xml
bool Game::readPreferences(folderStatus& status, Logger* logger)
{
	// For some reason preferences.xml is not valid xml and so we have to parse it with regex instead of xml
	std::string preferencesPath = (fs::path(status.gamePath) / "preferences.xml").string();
	if (status.steamVersion && status.preferencesPathBase != "CWD")
		preferencesPath = (fs::path(status.gamePath) / "bin" / status.folderVersion / "preferences.xml").string();

	if (fs::exists(fs::path(preferencesPath)))
	{
		std::ifstream ifs(preferencesPath);
		std::stringstream ss;
		ss << ifs.rdbuf();
		ifs.close();
		std::string pref = ss.str();

		static const std::regex versionRegex(R"regex(<clientVersion>([\s,0-9]*)<\/clientVersion>)regex");
		std::smatch match;

		if (std::regex_search(pref, match, versionRegex) && match.size() > 1)
		{
			status.gameVersion = match.str(1);
			std::replace(status.gameVersion.begin(), status.gameVersion.end(), ',', '.');
			status.gameVersion.erase(std::remove(status.gameVersion.begin(), status.gameVersion.end(), '\t'), status.gameVersion.end());
			return true;
		}
		else
		{
			logger->Error("Cannot find version string in preferences.xml.");
			return false;
		}
	}
	else
	{
		logger->Error("Cannot find preferences.xml for reading.");
		logger->Error(preferencesPath.c_str());
		return false;
	}
}

void Game::setReplaysFolder(folderStatus& status, Logger* logger) {
    if (status.replaysPathBase == "CWD")
    {
        status.replaysPath = (fs::path(status.gamePath) / status.replaysDirPath).string();
    }
    else if (status.replaysPathBase == "EXE_PATH")
    {
        if (status.steamVersion)
            // can be bin32 or bin64
            status.replaysPath = (fs::path(status.gamePath) / "bin" / status.gameVersion / "bin32" /
                                  status.replaysDirPath).string();
        else
            // can be bin32 or bin64
            status.replaysPath = (fs::path(status.gamePath) / "bin32" / status.replaysDirPath).string();
    }
	if (status.versionedReplays)
		// TODO
		;
}
