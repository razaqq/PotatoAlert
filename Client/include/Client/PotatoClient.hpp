// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Client/Game.hpp"
#include "Client/ReplayAnalyzer.hpp"
#include "Client/StatsParser.hpp"

#include "Core/DirectoryWatcher.hpp"
#include "Core/Singleton.hpp"

#include "ReplayParser/ReplayParser.hpp"

#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
#include <QNetworkReply>

#include <string>


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
	~PotatoClient() override = default;

	PA_SINGLETON(PotatoClient);

	void Init();
	void TriggerRun();
	DirectoryStatus CheckPath();

private:
	PotatoClient();
	void OnFileChanged(const std::string& file);

	void SendRequest(std::string_view request, MatchContext&& matchContext);
	void HandleReply(QNetworkReply* reply, auto& successHandler);
	void LookupResult(const std::string& url, const std::string& authToken, const MatchContext& matchContext);

	Core::DirectoryWatcher m_watcher;

	// std::string m_tempArenaInfo;
	std::string m_lastArenaInfoHash;
	DirectoryStatus m_dirStatus;
	ReplayAnalyzer m_replayAnalyzer;
	QNetworkAccessManager* m_networkAccessManager = new QNetworkAccessManager(this);

signals:
#pragma clang diagnostic push
#pragma ide diagnostic ignored "NotImplementedFunctions"
	void MatchReady(const StatsParser::Match& match);
	void MatchHistoryChanged();
	void MatchSummaryChanged(uint32_t id, const ReplaySummary& summary);
	void StatusReady(Status status, const std::string& statusText);
#pragma clang diagnostic pop
};

}  // namespace PotatoAlert::Client
