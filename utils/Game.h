// Copyright 2020 <github.com/razaqq>
#pragma once

#include <string>
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
	std::string replaysPath;
	std::string region;
	bool versionedReplays;
	bool steamVersion;
};

class Game
{
public:
	static folderStatus checkPath(const std::string& selectedPath, Logger* logger);
	static void getResFolderPath(folderStatus& status, Logger* logger);
	static bool readEngineConfig(folderStatus& status, Logger* logger);
	static bool readPreferences(folderStatus& status, Logger* logger);
	static void setReplaysFolder(folderStatus& status, Logger* logger);
};

};  // namespace PotatoAlert
