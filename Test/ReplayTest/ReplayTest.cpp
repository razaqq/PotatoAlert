// Copyright 2021 <github.com/razaqq>

#include "Core/Directory.hpp"
#include "Core/Log.hpp"
#include "Core/Process.hpp"
#include "Core/Result.hpp"
#include "Core/StandardPaths.hpp"
#include "Core/Version.hpp"

#include "ReplayParser/GameFiles.hpp"
#include "ReplayParser/ReplayParser.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>

#include <filesystem>
#include <numeric>
#include <optional>
#include <string>
#include <vector>


using PotatoAlert::Core::GetModuleRootPath;
using PotatoAlert::Core::Result;
using PotatoAlert::Core::Version;
using namespace PotatoAlert::ReplayParser;
namespace fs = std::filesystem;

namespace {

static fs::path GetReplay(std::string_view name)
{
	if (Result<fs::path> rootPath = GetModuleRootPath())
	{
		return rootPath.value().remove_filename() / "Replays" / name;
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
		PotatoAlert::Core::Log::Init(PotatoAlert::Core::AppDataPath("PotatoAlert") / "ReplayTest.log");
	}
};
CATCH_REGISTER_LISTENER(TestRunListener)

TEST_CASE( "ReplayTest" )
{
	const fs::path gameFilePath = GetModuleRootPath().value() / "ReplayVersions";

	{
		ReplayResult<Replay> res = Replay::FromFile(GetReplay("20201107_155356_PISC110-Venezia_19_OC_prey.wowsreplay"), gameFilePath);
		REQUIRE(res);
		REQUIRE(res->Meta.Name == "12x12");
		REQUIRE(res->Meta.DateTime == "07.11.2020 15:53:56");
		REQUIRE(res->Packets.size() == 153376);
		ReplayResult<ReplaySummary> result = res->Analyze();
		REQUIRE(result);
		REQUIRE(result->Outcome == MatchOutcome::Win);
	}

	{
		ReplayResult<Replay>  res = Replay::FromFile(GetReplay("20210914_212320_PRSC610-Smolensk_25_sea_hope.wowsreplay"), gameFilePath);
		REQUIRE(res);
		ReplayResult<ReplaySummary> result2 = res->Analyze();
		REQUIRE(result2);
		REQUIRE(result2->Outcome == MatchOutcome::Win);
	}

	{
		ReplayResult<Replay> res = Replay::FromFile(GetReplay("20210913_011502_PASD510-Somers_53_Shoreside.wowsreplay"), gameFilePath);
		REQUIRE(res);
		ReplayResult<ReplaySummary> result3 = res->Analyze();
		REQUIRE(result3);
		REQUIRE(result3->Outcome == MatchOutcome::Loss);
	}

	{
		ReplayResult<Replay> res = Replay::FromFile(GetReplay("20210912_002554_PRSB110-Sovetskaya-Rossiya_53_Shoreside.wowsreplay"), gameFilePath);
		REQUIRE(res);
		ReplayResult<ReplaySummary> result4 = res->Analyze();
		REQUIRE(result4);
		REQUIRE(result4->Outcome == MatchOutcome::Win);
	}

	{
		ReplayResult<Replay> res = Replay::FromFile(GetReplay("20210915_180756_PRSC610-Smolensk_35_NE_north_winter.wowsreplay"), gameFilePath);
		REQUIRE(res);
		ReplayResult<ReplaySummary> result5 = res->Analyze();
		REQUIRE(result5);
		REQUIRE(result5->Outcome == MatchOutcome::Loss);
	}

	{
		ReplayResult<Replay> res = Replay::FromFile(GetReplay("20201117_104604_PWSD508-Orkan_50_Gold_harbor.wowsreplay"), gameFilePath);
		REQUIRE(res.has_value());
		ReplayResult<ReplaySummary> result6 = res->Analyze();
		REQUIRE(result6);
		REQUIRE(result6->Outcome == MatchOutcome::Draw);
	}

	// 0.11.7
	{
		ReplayResult<Replay> res = Replay::FromFile(GetReplay("20220815_100927_PRSB518-Lenin_19_OC_prey.wowsreplay"), gameFilePath);
		REQUIRE(res.has_value());
		ReplayResult<ReplaySummary> result7 = res->Analyze();
		REQUIRE(result7);
		REQUIRE(result7->Outcome == MatchOutcome::Loss);
		REQUIRE((int)std::round(result7->DamageDealt) == 132976);
		REQUIRE((int)std::round(result7->DamageTaken) == 94536);
		REQUIRE((int)std::round(result7->DamageSpotting) == 17263);
		REQUIRE((int)std::round(result7->DamagePotential) == 1616598);
		REQUIRE(result7->Achievements.size() == 2);
		REQUIRE(result7->Ribbons.size() == 10);
		uint32_t count = std::accumulate(result7->Ribbons.begin(), result7->Ribbons.end(), (uint32_t)0, [](uint32_t current, const auto& ribbon)
		{
			return current + ribbon.second;
		});
		REQUIRE(count == 96);
	}

	// 12.6
	{
		ReplayResult<Replay> res = Replay::FromFile(GetReplay("20230723_181537_PWSD207-Grom_42_Neighbors.wowsreplay"), gameFilePath);
		REQUIRE(res.has_value());
		ReplayResult<ReplaySummary> result8 = res->Analyze();
		REQUIRE(result8);
		REQUIRE(result8->Outcome == MatchOutcome::Loss);
		REQUIRE((int)std::round(result8->DamageDealt) == 136864);
		REQUIRE((int)std::round(result8->DamageTaken) == 16603);
		REQUIRE((int)std::round(result8->DamageSpotting) == 5382);
		REQUIRE((int)std::round(result8->DamagePotential) == 583300);
		REQUIRE(result8->Achievements.size() == 2);
	}

	// 13.8.0
	{
		ReplayResult<Replay> res = Replay::FromFile(GetReplay("20240907_233354_PGSS208-U190_38_Canada.wowsreplay"), gameFilePath);
		REQUIRE(res.has_value());
		REQUIRE(res->Packets.size() == 123807);
	}

	{
		ReplayResult<Replay> res = Replay::FromFile(GetReplay("20240907_212325_PFSD018-L-Aventurier_41_Conquest.wowsreplay"), gameFilePath);
		REQUIRE(res.has_value());
		REQUIRE(res->Packets.size() == 61455);
	}
}

TEST_CASE( "ReplayGameFileTest" )
{
	const fs::path gameFilePath = GetModuleRootPath().value() / "ReplayVersions";

	const auto spec = ParseScripts(Version(0, 10, 8, 0), gameFilePath);
	REQUIRE(spec);
	REQUIRE(spec->size() == 13);
	REQUIRE(spec->at(0).Name == "Avatar");
	REQUIRE(spec->at(0).ClientProperties.size() == 17);
	REQUIRE(spec->at(0).BaseMethods.size() == 33);
	REQUIRE(spec->at(0).CellMethods.size() == 56);
	REQUIRE(spec->at(0).ClientMethods.size() == 153);
	REQUIRE(spec->at(0).AllProperties.size() == 20);
	REQUIRE(spec->at(0).ClientProperties.size() == 17);
	REQUIRE(spec->at(0).ClientPropertiesInternal.size() == 16);
	REQUIRE(spec->at(0).CellProperties.size() == 3);
	REQUIRE(spec->at(0).BaseProperties.size() == 1);
	REQUIRE(spec->at(0).Name == "Avatar");
}
