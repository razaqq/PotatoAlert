// Copyright 2022 <github.com/razaqq>

#include "Client/ReplayAnalyzer.hpp"

#include "Core/File.hpp"
#include "Core/String.hpp"
#include "GameFileUnpack/GameFileUnpack.hpp"

#include <algorithm>
#include <chrono>
#include <optional>
#include <ranges>
#include <string>


using namespace std::chrono_literals;
using namespace PotatoAlert::Core;
using PotatoAlert::GameFileUnpack::Unpacker;
using PotatoAlert::Client::ReplayAnalyzer;

bool ReplayAnalyzer::HasGameScripts(const Version& gameVersion) const
{
	return ReplayParser::HasGameScripts(gameVersion, m_scriptsSearchPaths);
}

bool ReplayAnalyzer::UnpackGameScripts(std::string_view dst, std::string_view pkgPath, std::string_view idxPath)
{
	const Unpacker unpacker(pkgPath, idxPath);
	if (!unpacker.Extract("scripts/", dst))
	{
		LOG_ERROR("Failed to unpack game files");
		return false;
	}
	return true;
}

void ReplayAnalyzer::OnFileChanged(const std::string& file)
{
	if (String::EndsWith(file, ".wowsreplay") && File::Exists(file) &&
		fs::path(file).filename().string() != "temp.wowsreplay")
	{
		LOG_TRACE("Replay file {} changed", file);
		AnalyzeReplay(file, 3s);
	}
}

void ReplayAnalyzer::AnalyzeReplay(std::string_view path, std::chrono::seconds readDelay)
{
	auto analyze = [this](std::string_view file, std::chrono::seconds readDelay) -> void
	{
		// this is honestly not ideal, but i don't see another way of fixing it
		LOG_TRACE("Analyzing replay file {} after {} delay...", file, readDelay);
		std::this_thread::sleep_for(readDelay);
		if (std::optional<ReplaySummary> res = ReplayParser::AnalyzeReplay(file, m_scriptsSearchPaths))
		{
			const ReplaySummary summary = res.value();
			const MatchHistory& matchHistory = m_services.Get<MatchHistory>();

			if (std::optional<MatchHistory::Entry> entry = matchHistory.GetEntry(summary.Hash))
			{
				matchHistory.SetAnalyzeResult(summary.Hash, summary);
				emit this->ReplaySummaryReady(entry.value().Id, summary);
			}
		}
	};

	m_futures.emplace_back(m_threadPool.Enqueue(analyze, std::string(path), readDelay));
}

void ReplayAnalyzer::AnalyzeDirectory(std::string_view directory)
{
	const MatchHistory& matchHistory = m_services.Get<MatchHistory>();

	auto entries = matchHistory.GetNonAnalyzedMatches();
	std::ranges::sort(entries,
	[](const MatchHistory::NonAnalyzedMatch& left, const MatchHistory::NonAnalyzedMatch& right)
	{
		return left.ReplayName < right.ReplayName;
	});

	for (const auto& entry : fs::recursive_directory_iterator(fs::path(directory)))
	{
		if (entry.is_regular_file() && entry.path().extension() == ".wowsreplay")
		{
			auto found = std::ranges::find_if(entries,
			[&](const MatchHistory::NonAnalyzedMatch& match)
			{
				const std::string fileName = String::ToLower(entry.path().filename().string());
				return String::ToLower(match.ReplayName) == fileName;
			});

			if (found != entries.end())
			{
				AnalyzeReplay(entry.path().string());
			}
		}
	}
}
