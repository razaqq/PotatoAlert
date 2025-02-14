// Copyright 2022 <github.com/razaqq>
#pragma once

#include "Client/Game.hpp"
#include "Client/ServiceProvider.hpp"

#include "Core/ThreadPool.hpp"
#include "Core/Version.hpp"

#include "GameFileUnpack/GameFileUnpack.hpp"

#include "ReplayParser/ReplayParser.hpp"

#include <QFileSystemWatcher>
#include <QString>

#include <chrono>
#include <filesystem>
#include <unordered_set>
#include <string>


namespace fs = std::filesystem;
using PotatoAlert::ReplayParser::ReplaySummary;
using PotatoAlert::ReplayParser::ReplayResult;

// these have to be declared, because ReplaySummaryReady will be emitted from
// another thread and the connection type will therefore be QueuedConnection
Q_DECLARE_METATYPE(uint32_t);
Q_DECLARE_METATYPE(ReplaySummary);

namespace PotatoAlert::Client {

class ReplayAnalyzer : public QObject
{
	Q_OBJECT

public:
	explicit ReplayAnalyzer(const ServiceProvider& serviceProvider)
		: m_services(serviceProvider)
	{
		qRegisterMetaType<uint32_t>("uint32_t");
		qRegisterMetaType<ReplaySummary>("ReplaySummary");
	}

	void AnalyzeDirectory(const std::filesystem::path& directory);
	void OnFileChanged(const std::filesystem::path& file);
	bool HasGameFiles(const Game::GameInfo& gameInfo) const;
	static Core::Result<void> UnpackGameFiles(const std::filesystem::path& dst, const std::filesystem::path& pkgPath, const std::filesystem::path& idxPath);

private:
	void AnalyzeReplay(const std::filesystem::path& path, std::chrono::seconds readDelay = std::chrono::seconds(0));

	const ServiceProvider& m_services;
	Core::ThreadPool m_threadPool;
	std::unordered_map<std::filesystem::path::string_type, std::future<void>> m_futures;

signals:
	void ReplaySummaryReady(uint32_t id, const ReplaySummary& summary) const;
};

}  // namespace PotatoAlert::Client
