// Copyright 2021 <github.com/razaqq>

#include "Core/Directory.hpp"
#include "Core/Version.hpp"
#include "ReplayParser/GameFiles.hpp"
#include "ReplayParser/ReplayParser.hpp"

#include "catch.hpp"
#include "win32.h"

#include <filesystem>
#include <iostream>
#include <string>


using PotatoAlert::Version;
using namespace PotatoAlert::ReplayParser;
namespace fs = std::filesystem;


static std::string GetReplay(std::string_view name)
{
	const auto rootPath = PotatoAlert::GetModuleRootPath();
	if (!rootPath.has_value())
	{
		exit(1);
	}

	return (fs::path(rootPath.value()).remove_filename() / "replays" / name).string();
}

TEST_CASE( "ReplayTest" )
{
	std::optional<Replay> res = Replay::FromFile(GetReplay("20201107_155356_PISC110-Venezia_19_OC_prey.wowsreplay"));
	REQUIRE(res.has_value());
	Replay replay = res.value();
	REQUIRE(replay.meta.name == "12x12");
	REQUIRE(replay.meta.dateTime == "07.11.2020 15:53:56");
	REQUIRE(replay.packets.empty());
	REQUIRE(replay.ReadPackets());
	REQUIRE(replay.packets.size() == 153376);
	MatchSummary result = replay.Analyze();
	REQUIRE((result && result.won));

	res = Replay::FromFile(GetReplay("20210914_212320_PRSC610-Smolensk_25_sea_hope.wowsreplay"));  // WIN
	REQUIRE(res.has_value());
	Replay replay2 = res.value();
	REQUIRE(replay2.ReadPackets());
	MatchSummary result2 = replay2.Analyze();
	REQUIRE((result2 && result2.won));

	res = Replay::FromFile(GetReplay("20210913_011502_PASD510-Somers_53_Shoreside.wowsreplay"));  // LOSS
	REQUIRE(res.has_value());
	Replay replay3 = res.value();
	REQUIRE(replay3.ReadPackets());
	MatchSummary result3 = replay3.Analyze();
	REQUIRE((result3 && !result3.won));


	res = Replay::FromFile(GetReplay("20210912_002554_PRSB110-Sovetskaya-Rossiya_53_Shoreside.wowsreplay"));  // WIN
	REQUIRE(res.has_value());
	Replay replay4 = res.value();
	REQUIRE(replay4.ReadPackets());
	MatchSummary result4 = replay4.Analyze();
	REQUIRE((result4 && result4.won));

	res = Replay::FromFile(GetReplay("20210915_180756_PRSC610-Smolensk_35_NE_north_winter.wowsreplay"));  // LOSS
	REQUIRE(res.has_value());
	Replay replay5 = res.value();
	REQUIRE(replay5.ReadPackets());
	MatchSummary result5 = replay5.Analyze();
	REQUIRE((result5 && !result5.won));
}

TEST_CASE( "ReplayGameFileTest" )
{
	const std::vector<EntitySpec> spec = ParseScripts(Version(0, 10, 8, 0));
	REQUIRE(spec.size() == 13);
	REQUIRE(spec[0].properties.size() == 17);
	REQUIRE(spec[0].baseMethods.size() == 33);
	REQUIRE(spec[0].cellMethods.size() == 56);
	REQUIRE(spec[0].clientMethods.size() == 153);
	REQUIRE(spec[0].internalProperties.size() == 16);
	REQUIRE(spec[0].name == "Avatar");
}
