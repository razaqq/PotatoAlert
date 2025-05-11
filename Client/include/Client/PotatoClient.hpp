// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Client/Config.hpp"
#include "Client/DatabaseManager.hpp"
#include "Client/DirectoryWatcher.hpp"
#include "Client/Game.hpp"
#include "Client/ReplayAnalyzer.hpp"
#include "Client/ServiceProvider.hpp"
#include "Client/StatsParser.hpp"
#include "Client/SysInfo.hpp"

#include "Core/Json.hpp"

#include "ReplayParser/Result.hpp"

#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
#include <QNetworkReply>

#include <filesystem>
#include <optional>
#include <string>
#include <vector>


using PotatoAlert::Client::Config;
using PotatoAlert::Client::Game::GameInfo;

namespace PotatoAlert::Client {

struct GameDirectory
{
	std::filesystem::path Path;
	std::string Status;
	std::optional<GameInfo> Info;
};

enum class Status
{
	Ready,
	Loading,
	Error
};

struct ClientOptions
{
	std::string_view SubmitUrl;
	std::string_view LookupUrl;
	int32_t TransferTimeout;
};

struct MatchContext
{
	std::string ArenaInfo;
	std::string PlayerName;
	std::string ShipIdent;
};

class PotatoClient : public QObject
{
	Q_OBJECT

public:
	explicit PotatoClient(ClientOptions&& clientOptions, const ServiceProvider& serviceProvider)
		: m_options(std::move(clientOptions)), m_services(serviceProvider), m_replayAnalyzer(serviceProvider.Get<ReplayAnalyzer>()) {}
	~PotatoClient() override = default;

	void Init();
	void TriggerRun();
	void ForceRun();
	void UpdateGameInstalls();
	[[nodiscard]] std::vector<GameDirectory> GetGameInstalls() const
	{
		return m_gameInfos;
	}

private:
	void OnFileChanged(const std::filesystem::path& file);
	void OnTempArenaInfoChanged(const std::filesystem::path& file, const GameInfo& game);
	void OnReplayChanged(const std::filesystem::path& file, const GameInfo& game);

	[[nodiscard]] std::filesystem::path GetGameFilePath(const GameInfo& game) const;
	[[nodiscard]] std::filesystem::path GetGameFilePath(const GameInfo& game, Core::Version version) const;

	[[nodiscard]] Core::JsonResult<std::string> BuildRequest(const std::string& region, const std::string& playerName, std::string_view metaString) const;
	void SendRequest(std::string_view requestString, MatchContext&& matchContext);
	void HandleReply(QNetworkReply* reply, auto& successHandler);
	void LookupResult(const std::string& url, const std::string& authToken, const MatchContext& matchContext);

	void DbAddMatch(std::string_view hash, const StatsParser::Match& match, const MatchContext& matchContext, std::string&& jsonString);

private:
	ClientOptions m_options;
	const ServiceProvider& m_services;
	DirectoryWatcher m_watcher;
	std::string m_lastArenaInfoHash;
	std::vector<GameDirectory> m_gameInfos;
	ReplayAnalyzer& m_replayAnalyzer;
	std::optional<SysInfo> m_sysInfo;
	QNetworkAccessManager* m_networkAccessManager = new QNetworkAccessManager();

signals:
	void MatchReady(const StatsParser::Match& match, const MatchContext& ctx);
	void MatchHistoryNewMatch(const DbMatch& match);
	void ReplaySummaryChanged(uint32_t id, const ReplaySummary& summary);
	void StatusReady(Status status, std::string_view statusText);
	void GameInfosChanged(std::span<const GameDirectory> infos);
};

}  // namespace PotatoAlert::Client
