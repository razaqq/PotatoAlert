// Copyright 2022 <github.com/razaqq>
#pragma once

#include "Core/Process.hpp"
#include "Core/Result.hpp"
#include "Core/StandardPaths.hpp"

#include <filesystem>
#include <string>


namespace fs = std::filesystem;
using PotatoAlert::Core::Result;

namespace PotatoAlert::Client {

struct AppDirectories
{
	explicit AppDirectories(std::string_view appName)
	{
		AppName = appName;
		AppDir = Core::AppDataPath(appName).make_preferred();
		MatchesDir = (AppDir / "Matches").make_preferred();
		ScreenshotsDir = (AppDir / "Screenshots").make_preferred();
		ReplayVersionsDir = (AppDir / "ReplayVersions").make_preferred();
		ConfigFile = (AppDir / "config.json").string();
		LogFile = (AppDir / std::format("{}.log", AppName)).string();

		auto createDir = [](const fs::path& dir) -> Result<void>
		{
			std::error_code ec;
			bool exists = fs::exists(dir, ec);
			if (ec)
				return PA_ERROR(ec);

			if (!exists)
			{
				fs::create_directories(dir, ec);
				if (ec)
					return PA_ERROR(ec);
			}

			return {};
		};

#define CHECK_ERROR(Result)                                                                                 \
		if (!(Result))                                                                                      \
		{                                                                                                   \
			LOG_ERROR("Failed to create app dir {}, reason: '{}'", MatchesDir, (Result).error().message()); \
			Core::ExitCurrentProcessWithError((Result).error().value());                               \
		}

		Result<void> result = createDir(MatchesDir);
		CHECK_ERROR(result)
		Result<void> result2 = createDir(ScreenshotsDir);
		CHECK_ERROR(result2)
		Result<void> result3 = createDir(ReplayVersionsDir);
		CHECK_ERROR(result3)

#undef CHECK_ERROR
	}

	std::string_view AppName;
	fs::path AppDir;

	fs::path MatchesDir;
	fs::path ScreenshotsDir;
	fs::path ReplayVersionsDir;
	std::string ConfigFile;
	std::string LogFile;
};

}  // namespace PotatoAlert::Client
