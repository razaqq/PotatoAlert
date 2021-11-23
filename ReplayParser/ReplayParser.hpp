// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Analyzer.hpp"
#include "File.hpp"
#include "Packets.hpp"
#include "ReplayMeta.hpp"

#include <optional>
#include <vector>


namespace PotatoAlert::ReplayParser {

class Replay
{
public:
	ReplayMeta meta;
	std::vector<PacketType> packets;
	std::vector<EntitySpec> specs;

	static std::optional<Replay> FromFile(std::string_view fileName);
	bool ReadPackets();
	[[nodiscard]] MatchSummary Analyze() const
	{
		return AnalyzePackets(packets, meta);
	}

private:
	std::span<std::byte> m_data;
	std::vector<std::byte> m_rawData;
};

void ProcessReplayDirectory();

}  // namespace PotatoAlert::ReplayParser
