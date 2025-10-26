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
	std::filesystem::path GamePath;
	Core::Version GameVersion;
	std::filesystem::path BinPath;
	std::filesystem::path IdxPath;
	std::filesystem::path PkgPath;
	bool VersionedReplays;
	std::vector<std::filesystem::path> ReplaysPaths;
	std::string Region;
};

namespace Detail {

struct GameCategoryT : std::error_category
{
	const char* name() const noexcept override;
	std::string message(int code) const override;
};
extern GameCategoryT const g_gameCategory;

}  // namespace Detail

std::vector<std::filesystem::path> GetDefaultGamePaths();
Core::Result<GameInfo> ReadGameInfo(const std::filesystem::path& selectedPath);
Core::Result<void> ReadRegion(GameInfo& gameInfo);

}  // namespace PotatoAlert::Client::Game
