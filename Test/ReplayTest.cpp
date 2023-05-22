// Copyright 2021 <github.com/razaqq>

#include "Core/Directory.hpp"
#include "Core/Result.hpp"
#include "Core/Version.hpp"
#include "ReplayParser/GameFiles.hpp"
#include "ReplayParser/ReplayParser.hpp"

#include "catch.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>


using PotatoAlert::Core::GetModuleRootPath;
using PotatoAlert::Core::Result;
using PotatoAlert::Core::Version;
using namespace PotatoAlert::ReplayParser;
namespace fs = std::filesystem;


static std::string GetReplay(std::string_view name)
{
	if (Result<fs::path> rootPath = GetModuleRootPath())
	{
		return (fs::path(rootPath.value()).remove_filename() / "replays" / name).string();
	}

	std::exit(1);
}

#if 0
TEST_CASE( "ReplayTest" )
{
	const fs::path root = GetModuleRootPath().value() / "ReplayVersions";

	ReplayResult<Replay> res = Replay::FromFile(GetReplay("20201107_155356_PISC110-Venezia_19_OC_prey.wowsreplay"));
	REQUIRE(res);
	REQUIRE(res->Meta.Name == "12x12");
	REQUIRE(res->Meta.DateTime == "07.11.2020 15:53:56");
	REQUIRE(res->Packets.empty());
	REQUIRE(res->ReadPackets({ root }));
	REQUIRE(res->Packets.size() == 153376);
	ReplayResult<ReplaySummary> result = res->Analyze();
	REQUIRE(result);
	REQUIRE(result->Outcome == MatchOutcome::Win);

	res = Replay::FromFile(GetReplay("20210914_212320_PRSC610-Smolensk_25_sea_hope.wowsreplay"));
	REQUIRE(res);
	REQUIRE(res->ReadPackets({ root }));
	ReplayResult<ReplaySummary> result2 = res->Analyze();
	REQUIRE(result2);
	REQUIRE(result2->Outcome == MatchOutcome::Win);

	res = Replay::FromFile(GetReplay("20210913_011502_PASD510-Somers_53_Shoreside.wowsreplay"));
	REQUIRE(res);
	REQUIRE(res->ReadPackets({ root }));
	ReplayResult<ReplaySummary> result3 = res->Analyze();
	REQUIRE(result3);
	REQUIRE(result3->Outcome == MatchOutcome::Loss);

	res = Replay::FromFile(GetReplay("20210912_002554_PRSB110-Sovetskaya-Rossiya_53_Shoreside.wowsreplay"));
	REQUIRE(res);
	REQUIRE(res->ReadPackets({ root }));
	ReplayResult<ReplaySummary> result4 = res->Analyze();
	REQUIRE(result4);
	REQUIRE(result4->Outcome == MatchOutcome::Win);

	res = Replay::FromFile(GetReplay("20210915_180756_PRSC610-Smolensk_35_NE_north_winter.wowsreplay"));
	REQUIRE(res);
	REQUIRE(res->ReadPackets({ root }));
	ReplayResult<ReplaySummary> result5 = res->Analyze();
	REQUIRE(result5);
	REQUIRE(result5->Outcome == MatchOutcome::Loss);

	res = Replay::FromFile(GetReplay("20201117_104604_PWSD508-Orkan_50_Gold_harbor.wowsreplay"));
	REQUIRE(res.has_value());
	REQUIRE(res->ReadPackets({ root }));
	ReplayResult<ReplaySummary> result6 = res->Analyze();
	REQUIRE(result6);
	REQUIRE(result6->Outcome == MatchOutcome::Draw);

	res = Replay::FromFile(GetReplay("20220815_100927_PRSB518-Lenin_19_OC_prey.wowsreplay"));
	REQUIRE(res.has_value());
	REQUIRE(res->ReadPackets({ root }));
	ReplayResult<ReplaySummary> result7 = res->Analyze();
	REQUIRE(result7);
	REQUIRE(result7->Outcome == MatchOutcome::Loss);
	REQUIRE((int)std::round(result7->DamageDealt) == 132976);
	REQUIRE((int)std::round(result7->DamageTaken) == 94536);
	REQUIRE((int)std::round(result7->DamageSpotting) == 17263);
	REQUIRE((int)std::round(result7->DamagePotential) == 1616598);
	REQUIRE(result7->Achievements.size() == 2);
	REQUIRE(result7->Ribbons.size() == 10);
	uint32_t count = std::accumulate(result7->Ribbons.begin(), result7->Ribbons.end(), 0, [](int current, const auto& ribbon)
	{
		return current + ribbon.second;
	});
	REQUIRE(count == 96);
}
#endif

TEST_CASE( "ReplayGameFileTest" )
{
	const fs::path root = GetModuleRootPath().value() / "ReplayVersions";

	const std::vector<EntitySpec> spec = ParseScripts(Version(0, 10, 8, 0), { root });
	REQUIRE(spec.size() == 13);
	REQUIRE(spec[0].Name == "Avatar");
	REQUIRE(spec[0].ClientProperties.size() == 17);
	REQUIRE(spec[0].BaseMethods.size() == 33);
	REQUIRE(spec[0].CellMethods.size() == 56);
	REQUIRE(spec[0].ClientMethods.size() == 153);
	REQUIRE(spec[0].AllProperties.size() == 20);
	REQUIRE(spec[0].ClientProperties.size() == 17);
	REQUIRE(spec[0].ClientPropertiesInternal.size() == 16);
	REQUIRE(spec[0].CellProperties.size() == 3);
	REQUIRE(spec[0].BaseProperties.size() == 1);
	REQUIRE(spec[0].Name == "Avatar");
}
