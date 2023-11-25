// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Client/Config.hpp"
#include "Client/DatabaseManager.hpp"
#include "Client/Game.hpp"
#include "Client/ReplayAnalyzer.hpp"
#include "Client/ServiceProvider.hpp"
#include "Client/StatsParser.hpp"

#include "Core/DirectoryWatcher.hpp"

#include "ReplayParser/ReplayParser.hpp"

#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
#include <QNetworkReply>

#include <filesystem>
#include <string>


using PotatoAlert::Client::Config;
using PotatoAlert::Client::Game::DirectoryStatus;
using PotatoAlert::Client::StatsParser::MatchContext;

namespace PotatoAlert::Client {

enum class Status
{
	Ready,
	Loading,
	Error
};

class PotatoClient : public QObject
{
	Q_OBJECT

public:
	explicit PotatoClient(const ServiceProvider& serviceProvider)
		: m_services(serviceProvider), m_replayAnalyzer(serviceProvider.Get<ReplayAnalyzer>()) {}
	~PotatoClient() override = default;

	void Init();
	void TriggerRun();
	void ForceRun();
	DirectoryStatus CheckPath();

private:
	void OnFileChanged(const std::filesystem::path& file);
	void SendRequest(std::string_view request, MatchContext&& matchContext);
	void HandleReply(QNetworkReply* reply, auto& successHandler);
	void LookupResult(const std::string& url, const std::string& authToken, const MatchContext& matchContext);

private:
	const ServiceProvider& m_services;
	Core::DirectoryWatcher m_watcher;
	std::string m_lastArenaInfoHash;
	DirectoryStatus m_dirStatus;
	ReplayAnalyzer& m_replayAnalyzer;
	QNetworkAccessManager* m_networkAccessManager = new QNetworkAccessManager();

	signals:
	void MatchReady(const StatsParser::MatchType& match);
	void MatchHistoryNewMatch(const Match& match);
	void ReplaySummaryChanged(uint32_t id, const ReplaySummary& summary);
	void StatusReady(Status status, std::string_view statusText);
	void DirectoryStatusChanged(const DirectoryStatus& status);
};

}  // namespace PotatoAlert::Client
