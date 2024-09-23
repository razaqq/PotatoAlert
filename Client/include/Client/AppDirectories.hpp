// Copyright 2022 <github.com/razaqq>
#pragma once

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
		AppDir = Core::AppDataPath(appName).make_preferred();
		MatchesDir = (AppDir / "Matches").make_preferred();
		ScreenshotsDir = (AppDir / "Screenshots").make_preferred();
		ReplayVersionsDir = (AppDir / "ReplayVersions").make_preferred();
		ConfigFile = (AppDir / "config.json").make_preferred();
		LogFile = (AppDir / fmt::format("{}.log", AppName)).make_preferred();
		DatabaseFile = (MatchesDir / "match_history.db").make_preferred();

		CreateAppDir(MatchesDir);
		CreateAppDir(ScreenshotsDir);
		CreateAppDir(ReplayVersionsDir);
	}

	std::string_view AppName;
	std::filesystem::path AppDir;

	std::filesystem::path MatchesDir;
	std::filesystem::path ScreenshotsDir;
	std::filesystem::path ReplayVersionsDir;
	std::filesystem::path ConfigFile;
	std::filesystem::path LogFile;
	std::filesystem::path DatabaseFile;

private:
	static void CreateAppDir(const std::filesystem::path& dir)
	{
		std::error_code ec;
		const bool exists = std::filesystem::exists(dir, ec);
		if (ec)
		{
			fmt::println(stderr, STR("Failed to check if app dir '{}' exists, reason: '{}'"), dir, ec);
			Core::ExitCurrentProcessWithError((uint32_t)ec.value());
		}

		if (!exists)
		{
			std::filesystem::create_directories(dir, ec);
			if (ec)
			{
				fmt::println(stderr, STR("Failed to create app dir '{}', reason: '{}'"), dir, ec);
				Core::ExitCurrentProcessWithError((uint32_t)ec.value());
			}
		}
	}
};

}  // namespace PotatoAlert::Client
