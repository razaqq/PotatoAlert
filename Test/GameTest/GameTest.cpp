// Copyright 2020 <github.com/razaqq>

#include "Client/Game.hpp"

#include "Core/Directory.hpp"
#include "Core/Log.hpp"
#include "Core/Process.hpp"
#include "Core/Result.hpp"
#include "Core/StandardPaths.hpp"
#include "Core/Version.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>

#include <filesystem>
#include <vector>


using PotatoAlert::Core::Result;
using PotatoAlert::Core::Version;
using namespace PotatoAlert::Client::Game;
namespace fs = std::filesystem;

namespace {

enum class Test
{
	NonSteamNonVersioned,
	NonSteamVersioned,
	NonSteamNonVersionedExe,
	SteamVersionedCwd
};

static fs::path GetGamePath(Test t)
{
	const auto rootPath = PotatoAlert::Core::GetModuleRootPath();
	if (!rootPath.has_value())
	{
		PotatoAlert::Core::ExitCurrentProcess(1);
	}

	switch (t)
	{
		case Test::NonSteamNonVersioned:
			return fs::path(rootPath.value()).remove_filename() / "GameDirectories" / "non_steam_non_versioned";
		case Test::NonSteamVersioned:
			return fs::path(rootPath.value()).remove_filename() / "GameDirectories" / "non_steam_versioned";
		case Test::NonSteamNonVersionedExe:
			return fs::path(rootPath.value()).remove_filename() / "GameDirectories" / "steam_non_versioned_exe";
		case Test::SteamVersionedCwd:
			return fs::path(rootPath.value()).remove_filename() / "GameDirectories" / "steam_versioned_cwd";
	}
	PotatoAlert::Core::ExitCurrentProcess(1);
}

}

class TestRunListener : public Catch::EventListenerBase
{
public:
	using Catch::EventListenerBase::EventListenerBase;

	void testRunStarting(Catch::TestRunInfo const&) override
	{
		PotatoAlert::Core::Log::Init(PotatoAlert::Core::AppDataPath("PotatoAlert") / "GameTest.log");
	}
};
CATCH_REGISTER_LISTENER(TestRunListener)

TEST_CASE( "GameTest" )
{
	{
		const fs::path p = GetGamePath(Test::NonSteamNonVersioned);
		const Result<GameInfo> info = ReadGameInfo(p);
		REQUIRE(info);
		REQUIRE(info->GameVersion == Version("0.9.4.0"));
		REQUIRE(info->BinPath == p / "bin" / "2666186");
		REQUIRE(info->IdxPath == info->BinPath / "idx");
		REQUIRE(info->PkgPath == p / "res_packages");
		REQUIRE_FALSE(info->VersionedReplays);
		REQUIRE(info->ReplaysPaths == std::vector<fs::path>{ p / "replays" });
		REQUIRE(info->Region == "eu");
	}

	{
		const fs::path p = GetGamePath(Test::NonSteamVersioned);
		const Result<GameInfo> info = ReadGameInfo(p);
		REQUIRE(info);
		REQUIRE(info->GameVersion == Version("0.9.4.0"));
		REQUIRE(info->BinPath == p / "bin" / "2666186");
		REQUIRE(info->IdxPath == info->BinPath / "idx");
		REQUIRE(info->PkgPath == p / "res_packages");
		REQUIRE(info->VersionedReplays);
		REQUIRE(info->ReplaysPaths == std::vector<fs::path>{ p / "replays" / "0.9.4.0" });
		REQUIRE(info->Region == "eu");
	}

	{
		const fs::path p = GetGamePath(Test::NonSteamNonVersionedExe);
		const Result<GameInfo> info = ReadGameInfo(p);
		REQUIRE(info);
		REQUIRE(info->GameVersion == Version("0.9.4.0"));
		REQUIRE(info->BinPath == p / "bin" / "1427460");
		REQUIRE(info->IdxPath == info->BinPath / "idx");
		REQUIRE(info->PkgPath == p / "res_packages");
		REQUIRE_FALSE(info->VersionedReplays);
		REQUIRE(info->Region == "eu");
		REQUIRE(info->ReplaysPaths == std::vector<fs::path>{
			p / "bin" / "1427460" / "bin32" / "replays",
			p / "bin" / "1427460" / "bin64" / "replays"
		});
		REQUIRE(info->Region == "eu");
	}

	{
		const fs::path p = GetGamePath(Test::SteamVersionedCwd);
		const Result<GameInfo> info = ReadGameInfo(p);
		REQUIRE(info);
		REQUIRE(info->GameVersion == Version("0.9.4.0"));
		REQUIRE(info->BinPath == p / "bin" / "1427460");
		REQUIRE(info->IdxPath == info->BinPath / "idx");
		REQUIRE(info->PkgPath == p / "res_packages");
		REQUIRE(info->VersionedReplays);
		REQUIRE(info->ReplaysPaths == std::vector<fs::path>{ p / "replays" / "0.9.4.0" });
		REQUIRE(info->Region == "eu");
	}
}
