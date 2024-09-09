// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Core/Version.hpp"

#include <filesystem>
#include <string>
#include <optional>
#include <vector>


namespace PotatoAlert::Client::Game {

struct DirectoryStatus
{
	std::filesystem::path gamePath;
	Core::Version gameVersion;
	std::filesystem::path binPath;
	std::filesystem::path idxPath;
	std::filesystem::path pkgPath;
	std::string preferencesPathBase;
	std::string directoryVersion;
	std::string replaysPathBase;
	std::filesystem::path replaysDirPath;
	std::vector<std::filesystem::path> replaysPath;
	std::string region;
	bool versionedReplays;
	std::string statusText = "No Game Directory Set";
	bool found = false;
};

bool CheckPath(const std::filesystem::path& selectedPath, DirectoryStatus& status);
bool GetBinPath(DirectoryStatus& status);
bool ReadEngineConfig(DirectoryStatus& status, const std::filesystem::path& resFolder);
bool ReadPreferences(DirectoryStatus& status, const std::filesystem::path& basePath);
void SetReplaysFolder(DirectoryStatus& status);
std::optional<std::filesystem::path> GetGamePath();

}  // namespace PotatoAlert::Client::Game
