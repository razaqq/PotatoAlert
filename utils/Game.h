// Copyright 2020 <github.com/razaqq>
#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include "Logger.h"


namespace fs = std::filesystem;

namespace PotatoAlert {

struct folderStatus
{
	std::string gamePath;
	std::string gameVersion;
	std::string resPath;
	std::string preferencesPathBase;
	std::string folderVersion;
	std::string replaysPathBase;
	std::string replaysDirPath;
	std::vector<std::string> replaysPath;
	std::string region;
	bool versionedReplays;
	bool steamVersion;
	std::string statusText;
	bool found = false;
};

class Game
{
public:
	static folderStatus checkPath(const std::string& selectedPath);
	static bool getResFolderPath(folderStatus& status);
	static bool readEngineConfig(folderStatus& status);
	static bool readPreferences(folderStatus& status);
	static void setReplaysFolder(folderStatus& status);
};

}  // namespace PotatoAlert