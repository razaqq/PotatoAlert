// Copyright 2022 <github.com/razaqq>
#pragma once

#include "Client/ServiceProvider.hpp"

#include "Core/ThreadPool.hpp"

#include "ReplayParser/ReplayParser.hpp"

#include <QFileSystemWatcher>
#include <QString>

#include <chrono>
#include <filesystem>
#include <unordered_set>
#include <string>


using namespace std::chrono_literals;
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
	ReplayAnalyzer(const ServiceProvider& serviceProvider, std::string_view gameFilePath)
		: m_services(serviceProvider), m_gameFilePath(gameFilePath)
	{
		qRegisterMetaType<uint32_t>("uint32_t");
		qRegisterMetaType<ReplaySummary>("ReplaySummary");
	}

	void AnalyzeDirectory(std::string_view directory);
	void OnFileChanged(const std::string& file);
	bool HasGameFiles(const Version& gameVersion) const;
	static ReplayResult<void> UnpackGameFiles(std::string_view dst, std::string_view pkgPath, std::string_view idxPath);

private:
	void AnalyzeReplay(std::string_view path, std::chrono::seconds readDelay = 0s);

	const ServiceProvider& m_services;
	Core::ThreadPool m_threadPool;
	std::unordered_map<std::string, std::future<void>> m_futures;
	std::string m_gameFilePath;

signals:
	void ReplaySummaryReady(uint32_t id, const ReplaySummary& summary) const;
};

}  // namespace PotatoAlert::Client
