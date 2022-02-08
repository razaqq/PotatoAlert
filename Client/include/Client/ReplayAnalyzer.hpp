// Copyright 2022 <github.com/razaqq>
#pragma once

#include "Client/MatchHistory.hpp"

#include "Core/ThreadPool.hpp"

#include "ReplayParser/ReplayParser.hpp"

#include <QFileSystemWatcher>
#include <QString>

#include <filesystem>
#include <unordered_set>
#include <string>


namespace fs = std::filesystem;
using PotatoAlert::ReplayParser::ReplaySummary;

// these have to be declared, because ReplaySummaryReady will be emitted from
// another thread and the connection type will therefore be QueuedConnection
Q_DECLARE_METATYPE(uint32_t);
Q_DECLARE_METATYPE(ReplaySummary);

namespace PotatoAlert::Client {

class ReplayAnalyzer : public QObject
{
	Q_OBJECT

public:
	ReplayAnalyzer()
	{
		qRegisterMetaType<uint32_t>("uint32_t");
		qRegisterMetaType<ReplaySummary>("ReplaySummary");
	}

	void AnalyzeDirectory(std::string_view directory);
	void OnFileChanged(const std::string& file);

private:
	void AnalyzeReplay(std::string_view path);

	std::unordered_set<std::string> m_analyzedReplays;
	Core::ThreadPool m_threadPool;
	std::vector<std::future<void>> m_futures;

signals:
	void ReplaySummaryReady(uint32_t id, const ReplaySummary& summary) const;
};

}  // namespace PotatoAlert::Client
