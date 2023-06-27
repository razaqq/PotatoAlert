// Copyright 2022 <github.com/razaqq>

#include "Core/Log.hpp"
#include "Core/Result.hpp"
#include "Core/StandardPaths.hpp"

#include "ReplayParser/ReplayParser.hpp"


using namespace PotatoAlert::Core;
using namespace PotatoAlert::ReplayParser;
using PotatoAlert::ReplayParser::ReplayResult;

int main(int argc, char* argv[])
{
	Log::Init((AppDataPath("PotatoAlert") / "StandaloneReplayParser.log").string());

	auto err = []()
	{
		printf("Press any key to continue...\n");
		getchar();
		return 1;
	};

	if (argc != 2)
	{
		LOG_ERROR("Arg 1 needs to be a replay file");
		return err();
	}

	PA_TRY_OR_ELSE(replay, Replay::FromFile(argv[1]),
	{
		LOG_ERROR(error);
		return err();
	});

	PA_TRYV_OR_ELSE(replay.ReadPackets({ AppDataPath("PotatoAlert") / "ReplayVersions" }),
	{
		LOG_ERROR( error);
		return err();
	});

	PA_TRY_OR_ELSE(summary, replay.Analyze(),
	{
		LOG_ERROR(error);
		return err();
	});

	LOG_INFO("DamageDealt: {} DamagePotential: {} DamageSpotting: {} DamageTaken: {}", summary.DamageDealt, summary.DamagePotential, summary.DamageSpotting, summary.DamageTaken);
	// LOG_INFO("---- DAMAGE  ----");
	// for (const auto& [type, dmg] : s.DamageDealt)
	// {
	// 	LOG_INFO("    {}: {}", GetName(type), dmg);
	// }
	LOG_INFO("---- RIBBONS ----");
	for (const auto& [ribbon, count] : summary.Ribbons)
	{
		LOG_INFO("    {}: {}", GetName(ribbon), count);
	}
	LOG_INFO("---- ACHIEVEMENTS ----");
	for (const auto& [achievement, count] : summary.Achievements)
	{
		LOG_INFO("    {}: {}", GetName(achievement), count);
	}

	printf("Press any key to continue...\n");
	getchar();
	return 0;
}
