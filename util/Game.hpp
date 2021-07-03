// Copyright 2020 <github.com/razaqq>
#pragma once

#include <string>
#include <vector>


namespace PotatoAlert::Game {

struct FolderStatus
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
	std::string statusText = "No Game Directory Set";
	bool found = false;
};

bool CheckPath(const std::string& selectedPath, FolderStatus& status);
bool GetResFolderPath(FolderStatus& status);
bool ReadEngineConfig(FolderStatus& status, const char* resFolder);
bool ReadPreferences(FolderStatus& status, const std::string& basePath);
void SetReplaysFolder(FolderStatus& status);

}  // namespace PotatoAlert::Game
