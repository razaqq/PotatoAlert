// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Core/Version.hpp"

#include <filesystem>
#include <string>
#include <optional>
#include <vector>


namespace fs = std::filesystem;

namespace PotatoAlert::Client::Game {

struct DirectoryStatus
{
	std::string gamePath;
	Version gameVersion;
	std::string resFolderPath;
	std::string preferencesPathBase;
	std::string directoryVersion;
	std::string replaysPathBase;
	std::string replaysDirPath;
	std::vector<std::string> replaysPath;
	std::string region;
	bool versionedReplays;
	std::string statusText = "No Game Directory Set";
	bool found = false;
};

bool CheckPath(const std::string& selectedPath, DirectoryStatus& status);
bool GetResFolderPath(DirectoryStatus& status);
bool ReadEngineConfig(DirectoryStatus& status, const char* resFolder);
bool ReadPreferences(DirectoryStatus& status, const std::string& basePath);
void SetReplaysFolder(DirectoryStatus& status);
std::optional<fs::path> GetGamePath();

}  // namespace PotatoAlert::Client::Game
