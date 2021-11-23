// Copyright 2021 <github.com/razaqq>

#include "Analyzer.hpp"

#include "Instrumentor.hpp"
#include "Version.hpp"

#include "GameFiles.hpp"

#include <any>
#include <span>
#include <unordered_map>
#include <vector>

MatchSummary AnalyzePackets(const std::vector<PacketType>& packets, const ReplayMeta& meta)
{
	PA_PROFILE_FUNCTION();

	struct
	{
		int8_t winningTeam = -1;
		int8_t playerTeam = -1;
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
			return MatchSummary{ false, false };
	}

	if (replayData.playerTeam == -1 || replayData.winningTeam == -1)
	{
		LOG_ERROR("Failed to determine match outcome: PT {} - WT {}", replayData.playerTeam, replayData.winningTeam);
		return MatchSummary{ false, false };
	}

	return MatchSummary{ replayData.playerTeam == replayData.winningTeam, true };
}
