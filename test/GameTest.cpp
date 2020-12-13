// Copyright 2020 <github.com/razaqq>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <Game.hpp>

using PotatoAlert::Game;
using PotatoAlert::folderStatus;
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
        case 0:
            return fs::current_path() / "gameDirectories" / "non_steam_non_versioned";
        case 1:
            return fs::current_path() / "gameDirectories" / "non_steam_versioned";
        case 2:
            return fs::current_path() / "gameDirectories" / "steam_non_versioned_exe";
        case 3:
            return fs::current_path() / "gameDirectories" / "steam_versioned_cwd";
        default:
            return fs::current_path();
    }
}

TEST_CASE( "GameTest_CheckPathTest" )
{
    folderStatus f1 = Game::checkPath(paths(nsnv).string());
    REQUIRE( f1.gamePath == paths(nsnv) );
    REQUIRE( f1.replaysPath == std::vector<std::string>{(paths(nsnv) / "replays").string()} );

	folderStatus f2 = Game::checkPath(paths(nsv).string());
    REQUIRE( f2.replaysPath == std::vector<std::string>{(paths(nsv) / "replays" / "0.9.4.0").string()} );

	folderStatus f3 = Game::checkPath(paths(snvexe).string());
    REQUIRE( f3.replaysPath == std::vector<std::string>{
        (paths(snvexe) / "bin" / "1427460" / "bin32" / "replays").string(),
        (paths(snvexe) / "bin" / "1427460" / "bin64" / "replays").string()
    } );

	folderStatus f4 = Game::checkPath(paths(svcwd).string());
    REQUIRE( f4.replaysPath == std::vector<std::string>{(paths(svcwd) / "replays" / "0.9.4.0").string()} );
}

TEST_CASE( "GameTest_ReadPreferencesTest" )
{
    folderStatus status = {
			paths(nsnv).string(),
            "", "", "cwd", "", "", "", {},
            "", false, false
    };
    REQUIRE( Game::readPreferences(status) );
    REQUIRE( status.gameVersion == "0.9.4.0" );
    REQUIRE( status.region == "eu" );
}

TEST_CASE( "GameTest_GetResFolderPathTest" )
{
    folderStatus status = {
			paths(nsnv).string(),
            "", "", "", "", "", "", {},
            "", false, false
    };
    REQUIRE( Game::getResFolderPath(status) );
    REQUIRE(status.resFolderPath == (paths(nsnv) / "bin" / "2666186").string() );

    status.gamePath = paths(snvexe).string();
    status.steamVersion = true;
    REQUIRE( Game::getResFolderPath(status) );
    REQUIRE( status.folderVersion == "1427460" );
    REQUIRE( status.resFolderPath == (paths(snvexe) / "bin" / "1427460").string() );
}

TEST_CASE( "GameTest_ReadEngineConfigTest" )
{
    // non steam non versioned
    folderStatus f1 = {
			paths(nsnv).string(),
            "", "", "", "", "", "", {},
            "", false, false
    };
    REQUIRE( Game::getResFolderPath(f1) );
    REQUIRE( Game::readEngineConfig(f1, "res") );
    REQUIRE( f1.replaysDirPath == "replays" );
    REQUIRE( f1.replaysPathBase == "cwd" );
    REQUIRE( !f1.versionedReplays );

    // steam non versioned
    folderStatus f2 = {
			paths(snvexe).string(),
            "", "", "", "", "", "", {},
            "", false, true
    };
    REQUIRE( Game::getResFolderPath(f2) );
    REQUIRE( Game::readEngineConfig(f2, "res") );
    REQUIRE( f2.replaysDirPath == "replays" );
    REQUIRE( f2.replaysPathBase == "exe_path" );
    REQUIRE( !f2.versionedReplays );
}

TEST_CASE( "GameTest_SetReplaysFolderTest" )
{
    folderStatus f1 = {
			paths(nsnv).string(),
            "0.9.4.0", (paths(nsnv) / "res").string(), "", "", "cwd", "replays",
            {},"eu", false, false
    };
    Game::setReplaysFolder(f1);
    REQUIRE( f1.replaysPath == std::vector<std::string>{(paths(nsnv) / "replays").string()} );

	folderStatus f2 = {
			paths(snvexe).string(),
            "0.9.4.0", (paths(snvexe) / "res").string(), "", "", "cwd", "replays",
            {}, "eu", false, true
    };
    Game::setReplaysFolder(f2);
    REQUIRE( f2.replaysPath == std::vector<std::string>{(paths(snvexe) / "replays").string()} );

	folderStatus f3 = {
			paths(snvexe).string(),
            "0.9.4.0", (paths(snvexe) / "res").string(), "", "1427460", "exe_path", "replays",
            {}, "eu", false, true
    };
    Game::setReplaysFolder(f3);
    REQUIRE( f3.replaysPath == std::vector<std::string>{
        (paths(snvexe) / "bin" / f3.folderVersion / "bin32" / "replays").string(),
        (paths(snvexe) / "bin" / f3.folderVersion / "bin64" / "replays").string()
    } );

	folderStatus f4 = {
			paths(svcwd).string(),
            "0.9.4.0", (paths(svcwd) / "res").string(), "", "1427460", "exe_path", "replays",
            {}, "eu", true, true
    };
    Game::setReplaysFolder(f4);
    REQUIRE( f4.replaysPath == std::vector<std::string>{
            (paths(svcwd) / "bin" / f4.folderVersion / "bin32" / "replays" / f4.gameVersion).string(),
            (paths(svcwd) / "bin" / f4.folderVersion / "bin64" / "replays" / f4.gameVersion).string()
    } );
}
