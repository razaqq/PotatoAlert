// Copyright 2022 <github.com/razaqq>
#pragma once

#include "Core/Directory.hpp"
#include "Core/Format.hpp"
#include "Core/Process.hpp"
#include "Core/Result.hpp"
#include "Core/StandardPaths.hpp"
#include "Core/String.hpp"

#include <filesystem>
#include <iostream>
#include <string>


using PotatoAlert::Core::Result;

namespace PotatoAlert::Client {

struct AppDirectories
{
public:
	explicit AppDirectories(std::string_view appName)
	{
		AppName = appName;

		PA_TRY_OR_ELSE(appDir, Core::AppDataPath(appName),
		{
			fmt::println(stderr, "Failed to get appDir: {}", error);
			Core::ExitCurrentProcess(1);
		});

		AppDir = appDir.make_preferred();
		MatchesDir = (AppDir / "Matches").make_preferred();
		ScreenshotsDir = (AppDir / "Screenshots").make_preferred();
		GameFilesDir = (AppDir / "GameFiles").make_preferred();
		ConfigFile = (AppDir / "config.json").make_preferred();
		LogFile = (AppDir / fmt::format("{}.log", AppName)).make_preferred();
		DatabaseFile = (MatchesDir / "match_history.db").make_preferred();

		PA_TRYV_OR_ELSE(CreateAppDir(MatchesDir),
		{
			fmt::println(stderr, "Failed to create MatchesDir {}: {}", MatchesDir, error);
			Core::ExitCurrentProcess(1);
		});
		PA_TRYV_OR_ELSE(CreateAppDir(ScreenshotsDir),
		{
			fmt::println(stderr, "Failed to create ScreenshotsDir {}: {}", ScreenshotsDir, error);
			Core::ExitCurrentProcess(1);
		});
		PA_TRYV_OR_ELSE(CreateAppDir(GameFilesDir),
		{
			fmt::println(stderr, "Failed to create GameFilesDir {}: {}", GameFilesDir, error);
			Core::ExitCurrentProcess(1);
		});
	}

	std::string_view AppName;
	std::filesystem::path AppDir;

	std::filesystem::path MatchesDir;
	std::filesystem::path ScreenshotsDir;
	std::filesystem::path GameFilesDir;
	std::filesystem::path ConfigFile;
	std::filesystem::path LogFile;
	std::filesystem::path DatabaseFile;

private:
	static Result<void> CreateAppDir(const std::filesystem::path& dir)
	{
		PA_TRY(exists, Core::PathExists(dir));
		if (!exists)
		{
			PA_TRYV(Core::CreatePath(dir));
		}
		return {};
	}
};

}  // namespace PotatoAlert::Client
