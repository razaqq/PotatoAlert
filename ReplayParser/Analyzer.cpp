// Copyright 2021 <github.com/razaqq>

#include "Core/Instrumentor.hpp"
#include "Core/Sha256.hpp"
#include "GameFiles.hpp"
#include "ReplayParser.hpp"

#include <any>
#include <unordered_map>
#include <vector>


using PotatoAlert::ReplayParser::Replay;
using namespace PotatoAlert::ReplayParser;
std::optional<ReplaySummary> Replay::Analyze() const
{
	PA_PROFILE_FUNCTION();

	struct
	{
		std::optional<int8_t> winningTeam = std::nullopt;
		std::optional<int8_t> playerTeam = std::nullopt;
	} replayData;

	for (const PacketType& packet : packets)
	{
		const bool success = std::visit([&replayData](auto&& packet) -> bool
		{
			using T = std::decay_t<decltype(packet)>;
			if constexpr (std::is_same_v<T, EntityMethodPacket>)
			{
				if (packet.methodName == "onBattleEnd")
				{
					if (auto wt = std::any_cast<int8_t>(&packet.values[0]))
					{
						replayData.winningTeam = *wt;
					}
					else
					{
						return false;
					}
				}
			}

			if constexpr (std::is_same_v<T, CellPlayerCreatePacket>)
			{
				if (packet.values.contains("teamId"))
				{
					if (auto t = std::any_cast<int8_t>(&packet.values.at("teamId")))
					{
						replayData.playerTeam = *t;
					}
					else
					{
						LOG_ERROR("Failed to get teamId for player");
						return false;
					}
				}
			}

			return true;
		}, packet);

		if (!success)
			return {};
	}

	MatchOutcome outcome;

	if (!replayData.playerTeam || !replayData.winningTeam)
	{
		LOG_TRACE("Failed to determine match outcome, pt {} WT {}", replayData.playerTeam.has_value(), replayData.winningTeam.has_value());
		outcome = MatchOutcome::Unknown;
	}
	else if (replayData.playerTeam.value() == replayData.winningTeam.value())
	{
		outcome = MatchOutcome::Win;
	}
	else if (replayData.winningTeam.value() == -1)
	{
		outcome = MatchOutcome::Draw;
	}
	else
	{
		outcome = MatchOutcome::Loss;
	}

	std::string hash;
	if (!Core::Sha256(metaString, hash))
		return {};

	return ReplaySummary{ hash, outcome };
}
