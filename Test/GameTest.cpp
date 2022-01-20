// Copyright 2020 <github.com/razaqq>

#include "Client/Game.hpp"
#include "Core/Directory.hpp"
#include "Core/Version.hpp"

#include "catch.hpp"
#include "win32.h"

#include <filesystem>


using PotatoAlert::Version;
using namespace PotatoAlert::Client::Game;
namespace fs = std::filesystem;

enum class Test
{
	nsnv,    // non steam non versioned
	nsv,     // non steam versioned
	snvexe,  // steam non versioned exe
	svcwd    // steam versioned cwd
};

static fs::path GetGamePath(Test t)
{
	const auto rootPath = PotatoAlert::GetModuleRootPath();
	if (!rootPath.has_value())
	{
		exit(1);
	}

	switch (t)
	{
		case Test::nsnv:
			return fs::path(rootPath.value()).remove_filename() / "gameDirectories" / "non_steam_non_versioned";
		case Test::nsv:
			return fs::path(rootPath.value()).remove_filename() / "gameDirectories" / "non_steam_versioned";
		case Test::snvexe:
			return fs::path(rootPath.value()).remove_filename() / "gameDirectories" / "steam_non_versioned_exe";
		case Test::svcwd:
			return fs::path(rootPath.value()).remove_filename() / "gameDirectories" / "steam_versioned_cwd";
	}
}

TEST_CASE( "GameTest_CheckPathTest" )
{
	FolderStatus f1;
	REQUIRE( CheckPath(GetGamePath(Test::nsnv).string(), f1) );
	REQUIRE( f1.gamePath == GetGamePath(Test::nsnv) );
	REQUIRE( f1.replaysPath == std::vector<std::string>{ (GetGamePath(Test::nsnv) / "replays").string() } );

	FolderStatus f2;
	REQUIRE( CheckPath(GetGamePath(Test::nsv).string(), f2) );
	REQUIRE( f2.replaysPath == std::vector<std::string>{ (GetGamePath(Test::nsv) / "replays" / "0.9.4.0").string()} );

	FolderStatus f3;
	REQUIRE( CheckPath(GetGamePath(Test::snvexe).string(), f3) );
	REQUIRE( f3.replaysPath == std::vector<std::string>{
		(GetGamePath(Test::snvexe) / "bin" / "1427460" / "bin32" / "replays").string(),
		(GetGamePath(Test::snvexe) / "bin" / "1427460" / "bin64" / "replays").string()
	});

	FolderStatus f4;
	REQUIRE( CheckPath(GetGamePath(Test::svcwd).string(), f4) );
	REQUIRE( f4.replaysPath == std::vector<std::string>{(GetGamePath(Test::svcwd) / "replays" / "0.9.4.0").string()} );
}

TEST_CASE( "GameTest_ReadPreferencesTest" )
{
	FolderStatus status = {
			GetGamePath(Test::nsnv).string(),
			Version(""), "", "cwd", "", "", "", {},
			"", false
	};
	REQUIRE( ReadPreferences(status, status.gamePath) );
	REQUIRE( status.gameVersion == Version("0.9.4.0") );
	REQUIRE( status.region == "eu" );
}

TEST_CASE( "GameTest_GetResFolderPathTest" )
{
	FolderStatus status = {
			GetGamePath(Test::nsnv).string(),
			Version(""), "", "", "", "", "", {},
			"", false
	};
	REQUIRE( GetResFolderPath(status) );
	REQUIRE(status.resFolderPath == (GetGamePath(Test::nsnv) / "bin" / "2666186").string() );

	status.gamePath = GetGamePath(Test::snvexe).string();
	REQUIRE( GetResFolderPath(status) );
	REQUIRE( status.folderVersion == "1427460" );
	REQUIRE( status.resFolderPath == (GetGamePath(Test::snvexe) / "bin" / "1427460").string() );
}

TEST_CASE( "GameTest_ReadEngineConfigTest" )
{
	// non steam non versioned
	FolderStatus f1 = {
			GetGamePath(Test::nsnv).string(),
			Version(""), "", "", "", "", "", {},
			"", false
	};
	REQUIRE( GetResFolderPath(f1) );
	REQUIRE( ReadEngineConfig(f1, "res") );
	REQUIRE( f1.replaysDirPath == "replays" );
	REQUIRE( f1.replaysPathBase == "cwd" );
	REQUIRE( !f1.versionedReplays );

	// steam non versioned
	FolderStatus f2 = {
			GetGamePath(Test::snvexe).string(),
			Version(""), "", "", "", "", "", {},
			"", false
	};
	REQUIRE( GetResFolderPath(f2) );
	REQUIRE( ReadEngineConfig(f2, "res") );
	REQUIRE( f2.replaysDirPath == "replays" );
	REQUIRE( f2.replaysPathBase == "exe_path" );
	REQUIRE( !f2.versionedReplays );
}

TEST_CASE( "GameTest_SetReplaysFolderTest" )
{
	FolderStatus f1 = {
			GetGamePath(Test::nsnv).string(),
			Version("0.9.4.0"), (GetGamePath(Test::nsnv) / "res").string(), "", "", "cwd", "replays",
			{}, "eu", false
	};
	SetReplaysFolder(f1);
	REQUIRE( f1.replaysPath == std::vector<std::string>{(GetGamePath(Test::nsnv) / "replays").string()} );

	FolderStatus f2 = {
			GetGamePath(Test::snvexe).string(),
			Version("0.9.4.0"), (GetGamePath(Test::snvexe) / "res").string(), "", "", "cwd", "replays",
			{}, "eu", false
	};
	SetReplaysFolder(f2);
	REQUIRE( f2.replaysPath == std::vector<std::string>{(GetGamePath(Test::snvexe) / "replays").string()} );

	FolderStatus f3 = {
			GetGamePath(Test::snvexe).string(),
			Version("0.9.4.0"), (GetGamePath(Test::snvexe) / "res").string(), "", "1427460", "exe_path", "replays",
			{}, "eu", false
	};
	SetReplaysFolder(f3);
	REQUIRE( f3.replaysPath == std::vector<std::string>{
		(GetGamePath(Test::snvexe) / "bin" / f3.folderVersion / "bin32" / "replays").string(),
		(GetGamePath(Test::snvexe) / "bin" / f3.folderVersion / "bin64" / "replays").string()
	});

	FolderStatus f4 = {
			GetGamePath(Test::svcwd).string(),
			Version("0.9.4.0"), (GetGamePath(Test::svcwd) / "res").string(), "", "1427460", "exe_path", "replays",
			{}, "eu", true
	};
	SetReplaysFolder(f4);
	REQUIRE( f4.replaysPath == std::vector<std::string>{
			(GetGamePath(Test::svcwd) / "bin" / f4.folderVersion / "bin32" / "replays" / f4.gameVersion.ToString()).string(),
			(GetGamePath(Test::svcwd) / "bin" / f4.folderVersion / "bin64" / "replays" / f4.gameVersion.ToString()).string()
	});
}
