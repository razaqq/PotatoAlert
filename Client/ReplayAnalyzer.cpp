// Copyright 2022 <github.com/razaqq>

#include "ReplayAnalyzer.hpp"

#include "Core/File.hpp"
#include "Core/String.hpp"

#include <qthread.h>
#include <string>
#include <vector>


using namespace PotatoAlert::Core;
using PotatoAlert::Client::ReplayAnalyzer;

void ReplayAnalyzer::OnFileChanged(const std::string& file)
{
	if (String::EndsWith(file, ".wowsreplay") && Core::File::Exists(file))
	{
		AnalyzeReplay(file);
	}
}

void ReplayAnalyzer::AnalyzeReplay(std::string_view path)
{
	auto analyze = [this](std::string_view file) -> void
	{
		if (std::optional<ReplaySummary> res = ReplayParser::AnalyzeReplay(file))
		{
			const ReplaySummary summary = res.value();
			if (std::optional<MatchHistory::Entry> entry = MatchHistory::Instance().GetEntry(summary.Hash))
			{
				MatchHistory::Instance().SetAnalyzeResult(summary.Hash, summary);
				emit this->ReplaySummaryReady(entry.value().Id, summary);
			}
		}
	};

	m_futures.emplace_back(m_threadPool.Enqueue(analyze, std::string(path)));
}

void ReplayAnalyzer::AnalyzeDirectory(std::string_view directory)
{
	auto entries = MatchHistory::Instance().GetNonAnalyzedMatches();
	std::ranges::sort(entries,
		[](const MatchHistory::NonAnalyzedMatch& left, const MatchHistory::NonAnalyzedMatch& right)
		{
			return left.ReplayName < right.ReplayName;
		}
	);

	for (const auto& entry : fs::recursive_directory_iterator(fs::path(directory)))
	{
		if (entry.is_regular_file() && entry.path().extension() == ".wowsreplay")
		{
			auto found = std::ranges::find_if(entries,
				[&](const MatchHistory::NonAnalyzedMatch& match)
				{
					std::string fileName = String::ToLower(entry.path().filename().string());
					return String::ToLower(match.ReplayName) == fileName;
				}
			);

			if (found != entries.end())
			{
				AnalyzeReplay(entry.path().string());
			}
		}
	}
}
