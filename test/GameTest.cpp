// Copyright 2020 <github.com/razaqq>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "Game.hpp"


using namespace PotatoAlert::Game;
namespace fs = std::filesystem;

enum test
{
	nsnv,    // non steam non versioned
	nsv,     // non steam versioned
	snvexe,  // steam non versioned exe
	svcwd    // steam versioned cwd
};

static fs::path paths(test t)
{
	switch (t) {
		case nsnv:
			return fs::current_path() / "gameDirectories" / "non_steam_non_versioned";
		case nsv:
			return fs::current_path() / "gameDirectories" / "non_steam_versioned";
		case snvexe:
			return fs::current_path() / "gameDirectories" / "steam_non_versioned_exe";
		case svcwd:
			return fs::current_path() / "gameDirectories" / "steam_versioned_cwd";
		default:
			return fs::current_path();
	}
}

TEST_CASE( "GameTest_CheckPathTest" )
{
	FolderStatus f1;
	REQUIRE( CheckPath(paths(nsnv).string(), f1) );
	REQUIRE( f1.gamePath == paths(nsnv) );
	REQUIRE( f1.replaysPath == std::vector<std::string>{(paths(nsnv) / "replays").string()} );

	FolderStatus f2;
	REQUIRE( CheckPath(paths(nsv).string(), f2) );
	REQUIRE( f2.replaysPath == std::vector<std::string>{(paths(nsv) / "replays" / "0.9.4.0").string()} );

	FolderStatus f3;
	REQUIRE( CheckPath(paths(snvexe).string(), f3) );
	REQUIRE( f3.replaysPath == std::vector<std::string>{
		(paths(snvexe) / "bin" / "1427460" / "bin32" / "replays").string(),
		(paths(snvexe) / "bin" / "1427460" / "bin64" / "replays").string()
	});

	FolderStatus f4;
	REQUIRE( CheckPath(paths(svcwd).string(), f4) );
	REQUIRE( f4.replaysPath == std::vector<std::string>{(paths(svcwd) / "replays" / "0.9.4.0").string()} );
}

TEST_CASE( "GameTest_ReadPreferencesTest" )
{
	FolderStatus status = {
			paths(nsnv).string(),
			"", "", "cwd", "", "", "", {},
			"", "", false
	};
	REQUIRE( ReadPreferences(status, status.gamePath) );
	REQUIRE( status.gameVersion == "0.9.4.0" );
	REQUIRE( status.region == "eu" );
}

TEST_CASE( "GameTest_GetResFolderPathTest" )
{
	FolderStatus status = {
			paths(nsnv).string(),
			"", "", "", "", "", "", {},
			"", "", false
	};
	REQUIRE( GetResFolderPath(status) );
	REQUIRE(status.resFolderPath == (paths(nsnv) / "bin" / "2666186").string() );

	status.gamePath = paths(snvexe).string();
	REQUIRE( GetResFolderPath(status) );
	REQUIRE( status.folderVersion == "1427460" );
	REQUIRE( status.resFolderPath == (paths(snvexe) / "bin" / "1427460").string() );
}

TEST_CASE( "GameTest_ReadEngineConfigTest" )
{
	// non steam non versioned
	FolderStatus f1 = {
			paths(nsnv).string(),
			"", "", "", "", "", "", {},
			"", "", false
	};
	REQUIRE( GetResFolderPath(f1) );
	REQUIRE( ReadEngineConfig(f1, "res") );
	REQUIRE( f1.replaysDirPath == "replays" );
	REQUIRE( f1.replaysPathBase == "cwd" );
	REQUIRE( !f1.versionedReplays );

	// steam non versioned
	FolderStatus f2 = {
			paths(snvexe).string(),
			"", "", "", "", "", "", {},
			"", "", false
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
			paths(nsnv).string(),
			"0.9.4.0", (paths(nsnv) / "res").string(), "", "", "cwd", "replays",
			{},"eu", "", false
	};
	SetReplaysFolder(f1);
	REQUIRE( f1.replaysPath == std::vector<std::string>{(paths(nsnv) / "replays").string()} );

	FolderStatus f2 = {
			paths(snvexe).string(),
			"0.9.4.0", (paths(snvexe) / "res").string(), "", "", "cwd", "replays",
			{}, "eu", "", false
	};
	SetReplaysFolder(f2);
	REQUIRE( f2.replaysPath == std::vector<std::string>{(paths(snvexe) / "replays").string()} );

	FolderStatus f3 = {
			paths(snvexe).string(),
			"0.9.4.0", (paths(snvexe) / "res").string(), "", "1427460", "exe_path", "replays",
			{}, "eu", "", false
	};
	SetReplaysFolder(f3);
	REQUIRE( f3.replaysPath == std::vector<std::string>{
		(paths(snvexe) / "bin" / f3.folderVersion / "bin32" / "replays").string(),
		(paths(snvexe) / "bin" / f3.folderVersion / "bin64" / "replays").string()
	});

	FolderStatus f4 = {
			paths(svcwd).string(),
			"0.9.4.0", (paths(svcwd) / "res").string(), "", "1427460", "exe_path", "replays",
			{}, "eu", "", true
	};
	SetReplaysFolder(f4);
	REQUIRE( f4.replaysPath == std::vector<std::string>{
			(paths(svcwd) / "bin" / f4.folderVersion / "bin32" / "replays" / f4.gameVersion).string(),
			(paths(svcwd) / "bin" / f4.folderVersion / "bin64" / "replays" / f4.gameVersion).string()
	});
}
