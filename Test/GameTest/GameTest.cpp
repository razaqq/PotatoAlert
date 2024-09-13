// Copyright 2020 <github.com/razaqq>

#include "Client/Game.hpp"

#include "Core/Directory.hpp"
#include "Core/Log.hpp"
#include "Core/Process.hpp"
#include "Core/StandardPaths.hpp"
#include "Core/Version.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>


using PotatoAlert::Core::Version;
using namespace PotatoAlert::Client::Game;
namespace fs = std::filesystem;

namespace {

static struct test_init
{
	test_init()
	{
		PotatoAlert::Core::Log::Init(PotatoAlert::Core::AppDataPath("PotatoAlert") / "GameTest.log");
	}
} test_init_instance;

enum class Test
{
	nsnv,    // non steam non versioned
	nsv,     // non steam versioned
	snvexe,  // steam non versioned exe
	svcwd    // steam versioned cwd
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
		case Test::nsnv:
			return fs::path(rootPath.value()).remove_filename() / "GameDirectories" / "non_steam_non_versioned";
		case Test::nsv:
			return fs::path(rootPath.value()).remove_filename() / "GameDirectories" / "non_steam_versioned";
		case Test::snvexe:
			return fs::path(rootPath.value()).remove_filename() / "GameDirectories" / "steam_non_versioned_exe";
		case Test::svcwd:
			return fs::path(rootPath.value()).remove_filename() / "GameDirectories" / "steam_versioned_cwd";
	}
	PotatoAlert::Core::ExitCurrentProcess(1);
}

}

TEST_CASE( "GameTest_CheckPathTest" )
{
	{
		DirectoryStatus d;
		REQUIRE( CheckPath(GetGamePath(Test::nsnv), d) );
		REQUIRE( d.gamePath == GetGamePath(Test::nsnv) );
		REQUIRE( d.replaysPath == std::vector<fs::path>{ GetGamePath(Test::nsnv) / "replays" } );
	}

	{
		DirectoryStatus d;
		REQUIRE( CheckPath(GetGamePath(Test::nsv), d) );
		REQUIRE(d.replaysPath == std::vector<fs::path>{ GetGamePath(Test::nsv) / "replays" / "0.9.4.0" });
	}

	{
		DirectoryStatus d;
		REQUIRE( CheckPath(GetGamePath(Test::snvexe), d) );
		REQUIRE(d.replaysPath == std::vector<fs::path>{
			GetGamePath(Test::snvexe) / "bin" / "1427460" / "bin32" / "replays",
			GetGamePath(Test::snvexe) / "bin" / "1427460" / "bin64" / "replays"
		});
	}

	{
		DirectoryStatus d;
		REQUIRE(CheckPath(GetGamePath(Test::svcwd), d));
		REQUIRE(d.replaysPath == std::vector<fs::path>{ GetGamePath(Test::svcwd) / "replays" / "0.9.4.0" });
	}
}

TEST_CASE( "GameTest_ReadPreferencesTest" )
{
	DirectoryStatus status =
	{
		GetGamePath(Test::nsnv),
		Version(""), "", "", "", "cwd", "", "", "", {},
		"", false
	};
	REQUIRE(ReadPreferences(status, status.gamePath));
	REQUIRE(status.gameVersion == Version("0.9.4.0"));
	REQUIRE(status.region == "eu");
}

TEST_CASE( "GameTest_GetResFolderPathTest" )
{
	DirectoryStatus status =
	{
		GetGamePath(Test::nsnv),
		Version(""), "", "", "", "", "", "", "", {},
		"", false
	};
	REQUIRE(GetBinPath(status));
	REQUIRE(status.binPath == GetGamePath(Test::nsnv) / "bin" / "2666186");

	status.gamePath = GetGamePath(Test::snvexe);
	REQUIRE(GetBinPath(status));
	REQUIRE(status.directoryVersion == "1427460");
	REQUIRE(status.binPath == GetGamePath(Test::snvexe) / "bin" / "1427460");
}

TEST_CASE( "GameTest_ReadEngineConfigTest" )
{
	{
		// non steam non versioned
		DirectoryStatus d = {
			GetGamePath(Test::nsnv),
			Version(""), "", "", "", "", "", "", "", {},
			"", false
		};
		REQUIRE(GetBinPath(d));
		REQUIRE(ReadEngineConfig(d, "res"));
		REQUIRE(d.replaysDirPath == "replays");
		REQUIRE(d.replaysPathBase == "cwd");
		REQUIRE(!d.versionedReplays);
	}

	{
		// steam non versioned
		DirectoryStatus d = {
			GetGamePath(Test::snvexe),
			Version(""), "", "", "", "", "", "", "", {},
			"", false
		};
		REQUIRE( GetBinPath(d) );
		REQUIRE( ReadEngineConfig(d, "res") );
		REQUIRE( d.replaysDirPath == "replays" );
		REQUIRE( d.replaysPathBase == "exe_path" );
		REQUIRE( !d.versionedReplays );
	}
}

TEST_CASE( "GameTest_SetReplaysFolderTest" )
{
	{
		DirectoryStatus d = {
			GetGamePath(Test::nsnv),
			Version("0.9.4.0"), GetGamePath(Test::nsnv) / "bin" / "1427460", "", "", "", "", "cwd", "replays",
			{}, "eu", false
		};
		SetReplaysFolder(d);
		REQUIRE(d.replaysPath == std::vector<fs::path>{ GetGamePath(Test::nsnv) / "replays" });
	}

	{
		DirectoryStatus d = {
			GetGamePath(Test::snvexe),
			Version("0.9.4.0"), GetGamePath(Test::snvexe) / "bin" / "1427460", "", "", "", "", "cwd", "replays",
			{}, "eu", false
		};
		SetReplaysFolder(d);
		REQUIRE(d.replaysPath == std::vector<fs::path>{ GetGamePath(Test::snvexe) / "replays" });
	}

	{
		DirectoryStatus d = {
			GetGamePath(Test::snvexe),
			Version("0.9.4.0"), GetGamePath(Test::snvexe) / "bin" / "1427460", "", "", "", "1427460", "exe_path", "replays",
			{}, "eu", false
		};
		SetReplaysFolder(d);
		REQUIRE(d.replaysPath == std::vector<fs::path>{
			GetGamePath(Test::snvexe) / "bin" / d.directoryVersion / "bin32" / "replays",
			GetGamePath(Test::snvexe) / "bin" / d.directoryVersion / "bin64" / "replays"
		});
	}

	{
		DirectoryStatus d = {
			GetGamePath(Test::svcwd),
			Version("0.9.4.0"), GetGamePath(Test::svcwd) / "bin" / "1427460", "", "", "", "1427460", "exe_path", "replays",
			{}, "eu", true
		};
		SetReplaysFolder(d);
		REQUIRE(d.replaysPath == std::vector<fs::path>{
				GetGamePath(Test::svcwd) / "bin" / d.directoryVersion / "bin32" / "replays" / d.gameVersion.ToString(),
				GetGamePath(Test::svcwd) / "bin" / d.directoryVersion / "bin64" / "replays" / d.gameVersion.ToString()
		});
	}
}
