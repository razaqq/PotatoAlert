// Copyright 2022 <github.com/razaqq>

#include "Client/DatabaseManager.hpp"
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
			const DatabaseManager& dbm = m_services.Get<DatabaseManager>();

			PA_TRY_OR_ELSE(match, dbm.GetMatch(summary.Hash),
			{
				LOG_ERROR("Failed to get match from match history: {}", error);
				return;
			});

			if (match)
			{
				PA_TRYV_OR_ELSE(dbm.SetMatchReplaySummary(summary.Hash, summary),
				{
					LOG_ERROR("Failed to set replay summary for match '{}': {}", summary.Hash, error);
					return;
				});
				emit ReplaySummaryReady(match.value().Id, summary);
			}
		}
	};

	m_futures.emplace_back(m_threadPool.Enqueue(analyze, std::string(path), readDelay));
}

void ReplayAnalyzer::AnalyzeDirectory(std::string_view directory)
{
	const DatabaseManager& dbm = m_services.Get<DatabaseManager>();

	PA_TRY_OR_ELSE(matches, dbm.GetNonAnalyzedMatches(),
	{
		LOG_ERROR("Failed to get non-analyzed matches from match history: {}", error);
		return;
	});

	std::ranges::sort(matches, [](const NonAnalyzedMatch& left, const NonAnalyzedMatch& right)
	{
		return left.ReplayName < right.ReplayName;
	});

	for (const auto& entry : fs::recursive_directory_iterator(fs::path(directory)))
	{
		if (entry.is_regular_file() && entry.path().extension() == ".wowsreplay")
		{
			auto found = std::ranges::find_if(matches, [&](const NonAnalyzedMatch& match)
			{
				const std::string fileName = String::ToLower(entry.path().filename().string());
				return String::ToLower(match.ReplayName) == fileName;
			});

			if (found != matches.end())
			{
				AnalyzeReplay(entry.path().string());
			}
		}
	}
}
