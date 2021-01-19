// Copyright 2020 <github.com/razaqq>
#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include "Logger.hpp"


namespace PotatoAlert::Game {

struct folderStatus
{
	std::string gamePath;
	std::string gameVersion;
	std::string resFolderPath;
	std::string preferencesPathBase;
	std::string folderVersion;
	std::string replaysPathBase;
	std::string replaysDirPath;
	std::vector<std::string> replaysPath;
	std::string overrideReplaysPath;
	std::string region;
	bool versionedReplays;
	bool steamVersion;
	std::string statusText;
	bool found = false;
};

folderStatus checkPath(const std::string& selectedPath);
bool getResFolderPath(folderStatus& status);
bool readEngineConfig(folderStatus& status, const char* resFolder);
bool readPreferences(folderStatus& status, const std::string& basePath);
void setReplaysFolder(folderStatus& status);

}  // namespace PotatoAlert::Game
