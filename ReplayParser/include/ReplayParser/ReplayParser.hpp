// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Core/Json.hpp"
#include "Packets.hpp"
#include "ReplayMeta.hpp"

#include <optional>
#include <span>
#include <string>
#include <vector>


namespace PotatoAlert::ReplayParser {

enum class MatchOutcome
{
	Win,
	Loss,
	Draw,
	Unknown
};
NLOHMANN_JSON_SERIALIZE_ENUM(MatchOutcome,
{
	{ MatchOutcome::Win, "win" },
	{ MatchOutcome::Loss, "loss" },
	{ MatchOutcome::Draw, "draw" },
	{ MatchOutcome::Unknown, "unknown" }
});

struct ReplaySummary
{
	std::string Hash;
	MatchOutcome Outcome;

	[[nodiscard]] std::string ToJson() const
	{
		const json j =
		{
			{ "outcome", Outcome }
		};

		return j.dump();
	}

	static ReplaySummary FromJson(std::string_view inJson)
	{
		json j;
		sax_no_exception sax(j);
		if (!json::sax_parse(inJson, &sax))
		{
			LOG_ERROR("Failed to parse replay summary info file as JSON.");
			return ReplaySummary{ "", MatchOutcome::Unknown };
		}

		MatchOutcome outcome = MatchOutcome::Unknown;
		if (j.contains("outcome"))
		{
			outcome = j["outcome"].get<MatchOutcome>();
		}

		return ReplaySummary{"", outcome };
	}
};

class Replay
{
public:
	std::string metaString;
	ReplayMeta meta;
	std::vector<PacketType> packets;
	std::vector<EntitySpec> specs;

	static std::optional<Replay> FromFile(std::string_view fileName);
	bool ReadPackets(const std::vector<fs::path>& scriptsSearchPaths);
	[[nodiscard]] std::optional<ReplaySummary> Analyze() const;

private:
	std::span<Byte> m_data;
	std::vector<Byte> m_rawData;
};

std::optional<ReplaySummary> AnalyzeReplay(std::string_view file, const std::vector<fs::path>& scriptsSearchPaths);
bool HasGameScripts(const Version& gameVersion, const std::vector<fs::path>& scriptsSearchPaths);

}  // namespace PotatoAlert::ReplayParser
