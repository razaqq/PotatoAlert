// Copyright 2021 <github.com/razaqq>

#include "Core/Directory.hpp"
#include "Core/Version.hpp"
#include "ReplayParser/GameFiles.hpp"
#include "ReplayParser/ReplayParser.hpp"

#include "catch.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>


using PotatoAlert::Core::GetModuleRootPath;
using PotatoAlert::Core::Version;
using namespace PotatoAlert::ReplayParser;
namespace fs = std::filesystem;


static std::string GetReplay(std::string_view name)
{
	if (std::optional<fs::path> rootPath = GetModuleRootPath())
	{
		return (fs::path(rootPath.value()).remove_filename() / "replays" / name).string();
	}

	std::exit(1);
}

static constexpr void ClearMem(Replay& replay)
{
	replay.packets.clear();
	replay.packets.shrink_to_fit();

	replay.specs.clear();
	replay.specs.shrink_to_fit();
}

TEST_CASE( "ReplayTest" )
{
	const fs::path root = GetModuleRootPath().value() / "ReplayVersions";

	std::optional<Replay> res = Replay::FromFile(GetReplay("20201107_155356_PISC110-Venezia_19_OC_prey.wowsreplay"));
	REQUIRE(res.has_value());
	Replay replay = res.value();
	REQUIRE(replay.meta.name == "12x12");
	REQUIRE(replay.meta.dateTime == "07.11.2020 15:53:56");
	REQUIRE(replay.packets.empty());
	REQUIRE(replay.ReadPackets({ root }));
	REQUIRE(replay.packets.size() == 153376);
	std::optional<ReplaySummary> result = replay.Analyze();
	REQUIRE(result);
	REQUIRE(result.value().Outcome == MatchOutcome::Win);
	ClearMem(replay);

	res = Replay::FromFile(GetReplay("20210914_212320_PRSC610-Smolensk_25_sea_hope.wowsreplay"));  // WIN
	REQUIRE(res.has_value());
	Replay replay2 = res.value();
	REQUIRE(replay2.ReadPackets({ root }));
	std::optional<ReplaySummary> result2 = replay2.Analyze();
	REQUIRE(result2);
	REQUIRE(result2.value().Outcome == MatchOutcome::Win);
	ClearMem(replay2);

	res = Replay::FromFile(GetReplay("20210913_011502_PASD510-Somers_53_Shoreside.wowsreplay"));  // LOSS
	REQUIRE(res.has_value());
	Replay replay3 = res.value();
	REQUIRE(replay3.ReadPackets({ root }));
	std::optional<ReplaySummary> result3 = replay3.Analyze();
	REQUIRE(result3);
	REQUIRE(result3.value().Outcome == MatchOutcome::Loss);
	ClearMem(replay3);


	res = Replay::FromFile(GetReplay("20210912_002554_PRSB110-Sovetskaya-Rossiya_53_Shoreside.wowsreplay"));  // WIN
	REQUIRE(res.has_value());
	Replay replay4 = res.value();
	REQUIRE(replay4.ReadPackets({ root }));
	std::optional<ReplaySummary> result4 = replay4.Analyze();
	REQUIRE(result4);
	REQUIRE(result4.value().Outcome == MatchOutcome::Win);
	ClearMem(replay4);

	res = Replay::FromFile(GetReplay("20210915_180756_PRSC610-Smolensk_35_NE_north_winter.wowsreplay"));  // LOSS
	REQUIRE(res.has_value());
	Replay replay5 = res.value();
	REQUIRE(replay5.ReadPackets({ root }));
	std::optional<ReplaySummary> result5 = replay5.Analyze();
	REQUIRE(result5);
	REQUIRE(result5.value().Outcome == MatchOutcome::Loss);
	ClearMem(replay5);

	res = Replay::FromFile(GetReplay("20201117_104604_PWSD508-Orkan_50_Gold_harbor.wowsreplay"));  // DRAW
	REQUIRE(res.has_value());
	Replay replay6 = res.value();
	REQUIRE(replay6.ReadPackets({ root }));
	std::optional<ReplaySummary> result6 = replay6.Analyze();
	REQUIRE(result6);
	REQUIRE(result6.value().Outcome == MatchOutcome::Draw);
}

TEST_CASE( "ReplayGameFileTest" )
{
	const fs::path root = GetModuleRootPath().value() / "ReplayVersions";

	const std::vector<EntitySpec> spec = ParseScripts(Version(0, 10, 8, 0), { root });
	REQUIRE(spec.size() == 13);
	REQUIRE(spec[0].properties.size() == 17);
	REQUIRE(spec[0].baseMethods.size() == 33);
	REQUIRE(spec[0].cellMethods.size() == 56);
	REQUIRE(spec[0].clientMethods.size() == 153);
	REQUIRE(spec[0].internalProperties.size() == 16);
	REQUIRE(spec[0].name == "Avatar");
}
