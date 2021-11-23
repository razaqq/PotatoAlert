// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Packets.hpp"
#include "ReplayMeta.hpp"


using namespace PotatoAlert;
using namespace ReplayParser;

struct MatchSummary
{
	bool won;

	explicit operator bool() const
	{
		return m_success;
	}

	bool m_success = true;
};

MatchSummary AnalyzePackets(const std::vector<PacketType>& packets, const ReplayMeta& meta);
