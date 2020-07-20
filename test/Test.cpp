// Copyright 2020 <github.com/razaqq>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <Game.h>

using PotatoAlert::Game;
using PotatoAlert::folderStatus;
namespace fs = std::filesystem;

const fs::path nsnvDir = fs::current_path() / "gameDirectories" / "non_steam_non_versioned";
const fs::path nsvDir = fs::current_path() / "gameDirectories" / "non_steam_versioned";
const fs::path snvexeDir = fs::current_path() / "gameDirectories" / "steam_non_versioned_exe";
const fs::path svcwdDir = fs::current_path() / "gameDirectories" / "steam_versioned_cwd";

fs::path testPaths(int id)
{
    switch (id) {
        case 0:
            return fs::current_path() / "gameDirectories" / "non_steam_non_versioned";
        case 1:
            return fs::current_path() / "gameDirectories" / "non_steam_versioned";
        case 2:
            return fs::current_path() / "gameDirectories" / "steam_non_versioned";
        case 3:
            return fs::current_path() / "gameDirectories" / "steam_versioned";
        default:
            return fs::current_path();
    }
}

TEST_CASE( "GameTest_CheckPathTest" )
{
    folderStatus folder = Game::checkPath(nsnvDir.string(), nullptr);
    REQUIRE( folder.gamePath == nsnvDir );
    REQUIRE( folder.replaysPath == std::vector<std::string>{(nsnvDir / "replays").string()} );

    folder = Game::checkPath(nsvDir.string(), nullptr);
    REQUIRE( folder.replaysPath == std::vector<std::string>{(nsvDir / "replays" / "0.9.4.0").string()} );

    folder = Game::checkPath(snvexeDir.string(), nullptr);
    REQUIRE( folder.replaysPath == std::vector<std::string>{
        (snvexeDir / "bin" / "1427460" / "bin32" / "replays").string(),
        (snvexeDir / "bin" / "1427460" / "bin64" / "replays").string()
    } );

    folder = Game::checkPath(svcwdDir.string(), nullptr);
    REQUIRE( folder.replaysPath == std::vector<std::string>{(svcwdDir / "replays" / "0.9.4.0").string()} );
}

TEST_CASE( "GameTest_ReadPreferencesTest" )
{
    folderStatus status = {
            nsnvDir.string(),
            "", "", "CWD", "", "", "", {},
            "", false, false
    };
    REQUIRE( Game::readPreferences(status, nullptr) );
    REQUIRE( status.gameVersion == "0.9.4.0" );
    REQUIRE( status.region == "eu" );
}

TEST_CASE( "GameTest_GetResFolderPathTest" )
{
    folderStatus status = {
            nsnvDir.string(),
            "", "", "", "", "", "", {},
            "", false, false
    };
    REQUIRE( Game::getResFolderPath(status, nullptr) );
    REQUIRE(status.resFolderPath == (nsnvDir / "bin" / "2666186" / "res").string() );

    status.gamePath = snvexeDir.string();
    status.steamVersion = true;
    REQUIRE( Game::getResFolderPath(status, nullptr) );
    REQUIRE( status.folderVersion == "1427460" );
    REQUIRE(status.resFolderPath == (snvexeDir / "bin" / "1427460" / "res").string() );
}

TEST_CASE( "GameTest_ReadEngineConfigTest" )
{
    // non steam non versioned
    folderStatus status = {
            nsnvDir.string(),
            "", "", "", "", "", "", {},
            "", false, false
    };
    REQUIRE( Game::getResFolderPath(status, nullptr) );
    REQUIRE( Game::readEngineConfig(status, nullptr) );
    REQUIRE( status.replaysDirPath == "replays" );
    REQUIRE( status.replaysPathBase == "CWD" );
    REQUIRE( !status.versionedReplays );

    // steam non versioned
    status = {
            snvexeDir.string(),
            "", "", "", "", "", "", {},
            "", false, true
    };
    REQUIRE( Game::getResFolderPath(status, nullptr) );
    REQUIRE( Game::readEngineConfig(status, nullptr) );
    REQUIRE( status.replaysDirPath == "replays" );
    REQUIRE( status.replaysPathBase == "EXE_PATH" );
    REQUIRE( !status.versionedReplays );
}

TEST_CASE( "GameTest_SetReplaysFolderTest" )
{
    folderStatus status = {
            nsnvDir.string(),
            "0.9.4.0", (nsnvDir / "res").string(), "", "", "CWD", "replays",
            {},"eu", false, false
    };
    Game::setReplaysFolder(status);
    REQUIRE( status.replaysPath == std::vector<std::string>{(nsnvDir / "replays").string()} );

    status = {
            snvexeDir.string(),
            "0.9.4.0", (snvexeDir / "res").string(), "", "", "CWD", "replays",
            {}, "eu", false, true
    };
    Game::setReplaysFolder(status);
    REQUIRE( status.replaysPath == std::vector<std::string>{(snvexeDir / "replays").string()} );

    status = {
            snvexeDir.string(),
            "0.9.4.0", (snvexeDir / "res").string(), "", "1427460", "EXE_PATH", "replays",
            {}, "eu", false, true
    };
    Game::setReplaysFolder(status);
    REQUIRE( status.replaysPath == std::vector<std::string>{
        (snvexeDir / "bin" / status.folderVersion / "bin32" / "replays").string(),
        (snvexeDir / "bin" / status.folderVersion / "bin64" / "replays").string()
    } );

    status = {
            svcwdDir.string(),
            "0.9.4.0", (svcwdDir / "res").string(), "", "1427460", "EXE_PATH", "replays",
            {}, "eu", true, true
    };
    Game::setReplaysFolder(status);
    REQUIRE( status.replaysPath == std::vector<std::string>{
            (svcwdDir / "bin" / status.folderVersion / "bin32" / "replays" / status.gameVersion).string(),
            (svcwdDir / "bin" / status.folderVersion / "bin64" / "replays" / status.gameVersion).string()
    } );
}
