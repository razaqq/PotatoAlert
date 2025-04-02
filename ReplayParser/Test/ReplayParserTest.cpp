// Copyright 2021 <github.com/razaqq>

#include "Core/Log.hpp"
#include "Core/Process.hpp"
#include "Core/Result.hpp"
#include "Core/StandardPaths.hpp"
#include "Core/Version.hpp"

#include "ReplayParser/ReplayParser.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>

#include <filesystem>
#include <numeric>
#include <optional>
#include <string>
#include <vector>


using PotatoAlert::Core::AppDataPath;
using PotatoAlert::Core::Result;
using PotatoAlert::Core::Version;
using namespace PotatoAlert::ReplayParser;
namespace fs = std::filesystem;

namespace {

static fs::path GetReplay(std::string_view name)
{
	return fs::current_path() / "Replays" / name;
}

static fs::path GetScriptsPath(Version version)
{
	return fs::current_path() / "ReplayVersions" / version.ToString(".", true) / "scripts";
}

}

class TestRunListener : public Catch::EventListenerBase
{
public:
	using Catch::EventListenerBase::EventListenerBase;

	void testRunStarting(Catch::TestRunInfo const&) override
	{
		PA_TRY_OR_ELSE(appData, AppDataPath("PotatoAlert"),
		{
			PotatoAlert::Core::ExitCurrentProcess(1);
		});
		PotatoAlert::Core::Log::Init(appData / "ReplayParserTest.log");
	}
};
CATCH_REGISTER_LISTENER(TestRunListener)

#if 0
TEST_CASE("Replay2Test")
{
	auto replay = Replay::Read(GetReplay("20241108_111729_PBSB503-Dreadnought_10_NE_big_race.wowsreplay"));
	auto specs = ParseScripts(GetScriptsPath(Version(13, 10, 0)));

	PacketParseContext ctx
	{
		.Specs = *specs,
		.Entities = {},
		.Version = Version(13, 10, 0),
	};

	auto parser = MakePacketParser(
		On<EntityCreatePacket>([]([[maybe_unused]] const EntityCreatePacket& p) -> void
		{
			printf("EntityCreatePacket");
		}),
		On<EntityMethodPacket>([]([[maybe_unused]] const EntityMethodPacket& p) -> void
		{
			printf("EntityMethodPacket");
		}),
		On<PlayerPositionPacket>([]([[maybe_unused]] const PlayerPositionPacket& p) -> void
		{
			printf("PlayerPositionPacket");
		}),
		On<EntityPropertyPacket>([]([[maybe_unused]] const EntityPropertyPacket& p) -> void
		{
			printf("EntityPropertyPacket");
		})
		//On<EntityPropertyPacket>([](const EntityPropertyPacket& p) -> void {})
	);

	PotatoAlert::Core::ByteReader<> reader(replay->m_decompressed);
	auto res = ParsePackets(reader, ctx, parser);
	REQUIRE(res.has_value());
}
#endif

TEST_CASE( "ReplayTest" )
{
	// 0.9.10
	{
		const auto scriptsPath = GetScriptsPath(Version(0, 9, 10));

		{
			ReplayResult<Replay> replay = Replay::FromFile(GetReplay("20201107_155356_PISC110-Venezia_19_OC_prey.wowsreplay"));
			REQUIRE(replay);
			REQUIRE(replay->Meta.Name == "12x12");
			REQUIRE(replay->Meta.DateTime == "07.11.2020 15:53:56");
			ReplayResult<std::vector<PacketType>> packets = replay->ParseAllPackets(scriptsPath);
			REQUIRE(packets);
			REQUIRE(packets->size() == 137416);  // with unknown 153376
			ReplayResult<ReplaySummary> summary = replay->Analyze(scriptsPath);
			REQUIRE(summary);
			REQUIRE(summary->Outcome == MatchOutcome::Win);
		}
		{
			ReplayResult<Replay> replay = Replay::FromFile(GetReplay("20201117_104604_PWSD508-Orkan_50_Gold_harbor.wowsreplay"));
			REQUIRE(replay.has_value());
			ReplayResult<ReplaySummary> summary = replay->Analyze(scriptsPath);
			REQUIRE(summary);
			REQUIRE(summary->Outcome == MatchOutcome::Draw);
		}
	}

	// 0.10.8
	{
		const auto scriptsPath = GetScriptsPath(Version(0, 10, 8));

		{
			ReplayResult<Replay> replay = Replay::FromFile(GetReplay("20210914_212320_PRSC610-Smolensk_25_sea_hope.wowsreplay"));
			REQUIRE(replay);
			ReplayResult<ReplaySummary> summary = replay->Analyze(scriptsPath);
			REQUIRE(summary);
			REQUIRE(summary->Outcome == MatchOutcome::Win);
		}
		{
			ReplayResult<Replay> replay = Replay::FromFile(GetReplay("20210913_011502_PASD510-Somers_53_Shoreside.wowsreplay"));
			REQUIRE(replay);
			ReplayResult<ReplaySummary> summary = replay->Analyze(scriptsPath);
			REQUIRE(summary);
			REQUIRE(summary->Outcome == MatchOutcome::Loss);
		}
		{
			ReplayResult<Replay> replay = Replay::FromFile(GetReplay("20210912_002554_PRSB110-Sovetskaya-Rossiya_53_Shoreside.wowsreplay"));
			REQUIRE(replay);
			ReplayResult<ReplaySummary> summary = replay->Analyze(scriptsPath);
			REQUIRE(summary);
			REQUIRE(summary->Outcome == MatchOutcome::Win);
		}
		{
			ReplayResult<Replay> replay = Replay::FromFile(GetReplay("20210915_180756_PRSC610-Smolensk_35_NE_north_winter.wowsreplay"));
			REQUIRE(replay);
			ReplayResult<ReplaySummary> summary = replay->Analyze(scriptsPath);
			REQUIRE(summary);
			REQUIRE(summary->Outcome == MatchOutcome::Loss);
		}
	}

	// 0.11.7
	{
		ReplayResult<Replay> replay = Replay::FromFile(GetReplay("20220815_100927_PRSB518-Lenin_19_OC_prey.wowsreplay"));
		REQUIRE(replay);
		ReplayResult<ReplaySummary> summary = replay->Analyze(GetScriptsPath(Version(0, 11, 7)));
		REQUIRE(summary);
		REQUIRE(summary->Outcome == MatchOutcome::Loss);
		REQUIRE((int)std::round(summary->DamageDealt) == 132976);
		REQUIRE((int)std::round(summary->DamageTaken) == 94536);
		REQUIRE((int)std::round(summary->DamageSpotting) == 17263);
		REQUIRE((int)std::round(summary->DamagePotential) == 1616598);
		REQUIRE(summary->Achievements.size() == 2);
		REQUIRE(summary->Ribbons.size() == 10);
		uint32_t count = std::accumulate(summary->Ribbons.begin(), summary->Ribbons.end(), (uint32_t)0, [](uint32_t current, const auto& ribbon)
		{
			return current + ribbon.second;
		});
		REQUIRE(count == 96);
	}

	// 12.6
	{
		ReplayResult<Replay> replay = Replay::FromFile(GetReplay("20230723_181537_PWSD207-Grom_42_Neighbors.wowsreplay"));
		REQUIRE(replay);
		ReplayResult<ReplaySummary> summary = replay->Analyze(GetScriptsPath(Version(12, 6, 0)));
		REQUIRE(summary);
		REQUIRE(summary->Outcome == MatchOutcome::Loss);
		REQUIRE((int)std::round(summary->DamageDealt) == 136864);
		REQUIRE((int)std::round(summary->DamageTaken) == 16603);
		REQUIRE((int)std::round(summary->DamageSpotting) == 5382);
		REQUIRE((int)std::round(summary->DamagePotential) == 583300);
		REQUIRE(summary->Achievements.size() == 2);
	}

	// 13.8.0
	{
		const auto scriptsPath = GetScriptsPath(Version(13, 8, 0));

		{
			ReplayResult<Replay> replay = Replay::FromFile(GetReplay("20240907_233354_PGSS208-U190_38_Canada.wowsreplay"));
			REQUIRE(replay);
			auto packets = replay->ParseAllPackets(scriptsPath);
			REQUIRE(packets);
			REQUIRE(packets->size() == 103737);  // with unknown 123807
		}
		{
			ReplayResult<Replay> replay = Replay::FromFile(GetReplay("20240907_212325_PFSD018-L-Aventurier_41_Conquest.wowsreplay"));
			REQUIRE(replay);
			auto packets = replay->ParseAllPackets(scriptsPath);
			REQUIRE(packets);
			REQUIRE(packets->size() == 52409);  // with unknown 61455
		}
	}

	// 13.10.0
	{
		const auto scriptsPath = GetScriptsPath(Version(13, 10, 0));

		{
			ReplayResult<Replay> replay = Replay::FromFile(GetReplay("20241107_152602_PWSD108-Oland_45_Zigzag.wowsreplay"));
			REQUIRE(replay);
			REQUIRE(replay);
			auto packets = replay->ParseAllPackets(scriptsPath);
			REQUIRE(packets);
			REQUIRE(packets->size() == 121222);  // with unknown 141911
			ReplayResult<ReplaySummary> result = replay->Analyze(scriptsPath);
			REQUIRE(result);
			REQUIRE(result->Outcome == MatchOutcome::Win);
		}
		{
			ReplayResult<Replay> replay = Replay::FromFile(GetReplay("20241108_110342_PASC003-Albany-1898_10_NE_big_race.wowsreplay"));
			REQUIRE(replay);
			auto packets = replay->ParseAllPackets(scriptsPath);
			REQUIRE(packets);
			REQUIRE(packets->size() == 107619);  // with unknown 126191
			ReplayResult<ReplaySummary> summary = replay->Analyze(scriptsPath);
			REQUIRE(summary);
			REQUIRE(summary->Outcome == MatchOutcome::Win);
		}
		{
			ReplayResult<Replay> replay = Replay::FromFile(GetReplay("20241108_111729_PBSB503-Dreadnought_10_NE_big_race.wowsreplay"));
			REQUIRE(replay);
			auto packets = replay->ParseAllPackets(scriptsPath);
			REQUIRE(packets);
			REQUIRE(packets->size() == 82132);  // with unknown 96437
			ReplayResult<ReplaySummary> summary = replay->Analyze(scriptsPath);
			REQUIRE(summary);
			REQUIRE(summary->Outcome == MatchOutcome::Win);
		}
	}
}

TEST_CASE( "ReplayGameFileTest" )
{
	const ReplayResult<std::vector<EntitySpec>> specs = Replay::ParseScripts(GetScriptsPath(Version(0, 10, 8, 0)));
	REQUIRE(specs);
	REQUIRE(specs->size() == 13);
	REQUIRE(specs->at(0).Name == "Avatar");
	REQUIRE(specs->at(0).ClientProperties.size() == 17);
	REQUIRE(specs->at(0).BaseMethods.size() == 33);
	REQUIRE(specs->at(0).CellMethods.size() == 56);
	REQUIRE(specs->at(0).ClientMethods.size() == 153);
	REQUIRE(specs->at(0).AllProperties.size() == 20);
	REQUIRE(specs->at(0).ClientProperties.size() == 17);
	REQUIRE(specs->at(0).ClientPropertiesInternal.size() == 16);
	REQUIRE(specs->at(0).CellProperties.size() == 3);
	REQUIRE(specs->at(0).BaseProperties.size() == 1);
	REQUIRE(specs->at(0).Name == "Avatar");
}
