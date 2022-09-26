// Copyright 2022 <github.com/razaqq>

#include "Core/Log.hpp"
#include "Core/StandardPaths.hpp"

#include "ReplayParser/ReplayParser.hpp"


using namespace PotatoAlert::Core;
using namespace PotatoAlert::ReplayParser;

int main(int argc, char* argv[])
{
	Log::Init((AppDataPath("PotatoAlert") / "StandaloneReplayParser.log").string());

	if (argc != 2)
	{
		LOG_ERROR("Arg 1 needs to be a replay file");
		goto ERR;
	}

	if (std::optional<Replay> replay = Replay::FromFile(argv[1]))
	{
		if (!replay->ReadPackets({ AppDataPath("PotatoAlert") / "ReplayVersions" }))
		{
			LOG_ERROR("Failed to read packets");
			goto ERR;
		}
		if (std::optional<ReplaySummary> summary = replay->Analyze())
		{
			ReplaySummary s = summary.value();
			LOG_INFO("DamageDealt: {} DamagePotential: {} DamageSpotting: {} DamageTaken: {}", s.DamageDealt, s.DamagePotential, s.DamageSpotting, s.DamageTaken);
			// LOG_INFO("---- DAMAGE  ----");
			// for (const auto& [type, dmg] : s.DamageDealt)
			// {
			// 	LOG_INFO("    {}: {}", GetName(type), dmg);
			// }
			LOG_INFO("---- RIBBONS ----");
			for (const auto& [ribbon, count] : s.Ribbons)
			{
				LOG_INFO("    {}: {}", GetName(ribbon), count);
			}
			LOG_INFO("---- ACHIEVEMENTS ----");
			for (const auto& [achievement, count] : s.Achievements)
			{
				LOG_INFO("    {}: {}", GetName(achievement), count);
			}
			// printf("%s", summary->ToJson().c_str());
			goto OK;
		}
		else
		{
			LOG_ERROR("Failed to analyze replay");
			goto ERR;
		}
	}
	else
	{
		LOG_ERROR("Failed to parse replay from file '{}'", argv[1]);
		goto ERR;
	}

ERR:
	printf("Press any key to continue...\n");
	getchar();
	return 1;

OK:
	printf("Press any key to continue...\n");
	getchar();
	return 0;
}
