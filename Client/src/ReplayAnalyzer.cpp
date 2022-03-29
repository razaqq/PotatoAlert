// Copyright 2022 <github.com/razaqq>

#include "Client/ReplayAnalyzer.hpp"

#include "Core/File.hpp"
#include "Core/String.hpp"
#include "GameFileUnpack/GameFileUnpack.hpp"

#include <algorithm>
#include <optional>
#include <string>


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
	if (String::EndsWith(file, ".wowsreplay") && 
		File::Exists(file) &&
		fs::path(file).filename().string() != "temp.wowsreplay")
	{
		AnalyzeReplay(file);
	}
}

void ReplayAnalyzer::AnalyzeReplay(std::string_view path)
{
	auto analyze = [this](std::string_view file) -> void
	{
		if (std::optional<ReplaySummary> res = ReplayParser::AnalyzeReplay(file, m_scriptsSearchPaths))
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
	std::sort(entries.begin(), entries.end(),
		[](const MatchHistory::NonAnalyzedMatch& left, const MatchHistory::NonAnalyzedMatch& right)
		{
			return left.ReplayName < right.ReplayName;
		}
	);

	for (const auto& entry : fs::recursive_directory_iterator(fs::path(directory)))
	{
		if (entry.is_regular_file() && entry.path().extension() == ".wowsreplay")
		{
			auto found = std::find_if(entries.begin(), entries.end(),
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
