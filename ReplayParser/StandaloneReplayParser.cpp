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

	auto section = [](std::string_view title = "")
	{
		LOG_INFO("{:-^21}", title);
	};

	auto err = []() -> int
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

	PA_TRYV_OR_ELSE(replay.ReadPackets((AppDataPath("PotatoAlert") / "ReplayVersions").string()),
	{
		LOG_ERROR(error);
		return err();
	});

	PA_TRY_OR_ELSE(summary, replay.Analyze(),
	{
		LOG_ERROR(error);
		return err();
	});

	section("GENERAL");
	LOG_INFO("DamageDealt:     {}", summary.DamageDealt);
	LOG_INFO("DamagePotential: {}", summary.DamagePotential);
	LOG_INFO("DamageSpotting:  {}", summary.DamageSpotting);
	LOG_INFO("DamageTaken:     {}", summary.DamageTaken);

	constexpr auto toString = [](MatchOutcome outcome) -> std::string_view
	{
		switch (outcome)
		{
			case MatchOutcome::Win:
				return "Win";
			case MatchOutcome::Loss:
				return "Loss";
			case MatchOutcome::Draw:
				return "Draw";
			case MatchOutcome::Unknown:
				return "Unknown";
		}
	};
	LOG_INFO("Outcome: {}", toString(summary.Outcome));

	section("RIBBONS");
	for (const auto& [ribbon, count] : summary.Ribbons)
	{
		LOG_INFO("    {}: {}", GetName(ribbon), count);
	}

	section("ACHIEVEMENTS");
	for (const auto& [achievement, count] : summary.Achievements)
	{
		LOG_INFO("    {}: {}", GetName(achievement), count);
	}

	printf("Press any key to continue...\n");
	getchar();
	return 0;
}
