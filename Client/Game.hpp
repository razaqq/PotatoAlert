// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Version.hpp"

#include <filesystem>
#include <string>
#include <optional>
#include <vector>


namespace fs = std::filesystem;

namespace PotatoAlert::Game {

struct FolderStatus
{
	std::string gamePath;
	Version gameVersion;
	std::string resFolderPath;
	std::string preferencesPathBase;
	std::string folderVersion;
	std::string replaysPathBase;
	std::string replaysDirPath;
	std::vector<std::string> replaysPath;
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
std::optional<fs::path> GetGamePath();

}  // namespace PotatoAlert::Game
