// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Core/Result.hpp"
#include "Core/Version.hpp"

#include <filesystem>
#include <string>
#include <vector>


namespace PotatoAlert::Client::Game {

enum class GameError
{
	DirectoryNotFound,
	PreferencesXmlMissing,
	PreferencesXmlFailedToMap,
	PreferencesXmlMissingVersion,
	PreferencesXmlMissingRegion,
	PreferencesXmlInvalidRegion,
	BinPathMissing,
	BinPathFailedToReadPeVersion,
	BinPathFailedToDetermine,
	EngineConfigXmlMissing,
	EngineConfigXmlFailedLoading,
	EngineConfigXmlEmpty,
	EngineConfigXmlMissingReplays,
	EngineConfigXmlMissingDirPath,
	EngineConfigXmlMissingPathBase,
	EngineConfigXmlMissingVersioned,
	EngineConfigXmlMissingPreferences,
	EngineConfigXmlMissingPreferencesPathBase,
	EngineConfigXmlVersionedInvalid,
};

struct GameInfo
{
	Core::Version GameVersion;
	std::filesystem::path BinPath;
	std::filesystem::path IdxPath;
	std::filesystem::path PkgPath;
	bool VersionedReplays;
	std::vector<std::filesystem::path> ReplaysPaths;
	std::string Region;
};

std::vector<std::filesystem::path> GetDefaultGamePaths();
Core::Result<GameInfo> ReadGameInfo(const std::filesystem::path& selectedPath);

}  // namespace PotatoAlert::Client::Game
