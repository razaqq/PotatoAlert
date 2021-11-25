// Copyright 2020 <github.com/razaqq>

#include "PotatoClient.hpp"

#include "Config.hpp"
#include "Game.hpp"
#include "Hash.hpp"
#include "Json.hpp"
#include "Log.hpp"
#include "Serializer.hpp"
#include "StatsParser.hpp"

#include <QFileSystemWatcher>
#include <QObject>
#include <QString>
#include <QTableWidgetItem>
#include <QUrl>
#include <QWebSocket>

#include <chrono>
#include <filesystem>
#include <format>
#include <optional>
#include <string>
#include <thread>
#include <utility>


using PotatoAlert::PotatoClient;
namespace fs = std::filesystem;

static const char* g_wsAddress = "ws://www.perry-swift.de:33333";
// static const char* g_wsAddress = "ws://192.168.178.36:10000";

void PotatoClient::Init()
{
	emit this->StatusReady(Status::Ready, "Ready");

	// handle connection
	connect(this->m_socket, &QWebSocket::connected, [this]
	{
		LOG_TRACE("Websocket open, sending arena info...");
		this->m_socket->sendTextMessage(this->m_tempArenaInfo);
	});

	// handle error
	connect(this->m_socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), [this](QAbstractSocket::SocketError error)
	{
		switch (error)
		{
			case QAbstractSocket::ConnectionRefusedError:
				emit this->StatusReady(Status::Error, "Connection Refused");
				break;
			case QAbstractSocket::SocketTimeoutError:
				emit this->StatusReady(Status::Error, "Socket Timeout");
				break;
			case QAbstractSocket::HostNotFoundError:
				emit this->StatusReady(Status::Error, "Host Not Found");
				break;
			case QAbstractSocket::NetworkError:
				// TODO: this is a shit way of doing this, maybe create a Qt bug report?
				if (this->m_socket->errorString().contains("timed", Qt::CaseInsensitive))
					emit this->StatusReady(Status::Error, "Connection Timeout");
				else
					emit this->StatusReady(Status::Error, "Network Error");
				break;
			case QAbstractSocket::RemoteHostClosedError:
				emit this->StatusReady(Status::Error, "Host Closed Conn.");
				return;
			default:
				emit this->StatusReady(Status::Error, "Websocket Error");
				break;
		}
		LOG_ERROR(this->m_socket->errorString().toStdString());
	});

	connect(this->m_socket, &QWebSocket::textMessageReceived, this, &PotatoClient::OnResponse);
	connect(this->m_watcher, &QFileSystemWatcher::directoryChanged, this, &PotatoClient::OnDirectoryChanged);

	this->TriggerRun();
}

// triggers a run with the current replays folders
void PotatoClient::TriggerRun()
{
	for (auto& path : this->m_watcher->directories())
		this->OnDirectoryChanged(path);
}

// triggered whenever a file gets modified in a replays path
void PotatoClient::OnDirectoryChanged(const QString& path)
{
	LOG_TRACE("Directory changed.");

	const std::string filePath = std::format("{}\\tempArenaInfo.json", path.toStdString());

	if (fs::exists(filePath))
	{
		// read arena info from file
		auto arenaInfo = PotatoClient::ReadArenaInfo(filePath);
		if (!arenaInfo.has_value())
		{
			LOG_ERROR("Failed to read arena info from file.");
			emit this->StatusReady(Status::Error, "Reading ArenaInfo");
			return;
		}

		json j;
		sax_no_exception sax(j);
		if (!json::sax_parse(arenaInfo.value(), &sax))
		{
			LOG_ERROR("Failed to Parse arena info file as JSON.");
			emit this->StatusReady(Status::Error, "JSON Parse Error");
			return;
		}

		LOG_TRACE("ArenaInfo read from file. Content: {}", j.dump());

		// add region
		j["region"] = this->m_folderStatus.region;

		// set stats mode from config
		switch (PotatoConfig().Get<StatsMode>("stats_mode"))
		{
			case StatsMode::Current:
				if (j.contains("matchGroup"))
					j["StatsMode"] = j["matchGroup"];
				else
					j["StatsMode"] = "pvp";
				break;
			case StatsMode::Pvp:
				j["StatsMode"] = "pvp";
				break;
			case StatsMode::Ranked:
				j["StatsMode"] = "ranked";
				break;
			case StatsMode::Clan:
				j["StatsMode"] = "clan";
				break;
		}

		// make sure we don't pull the same match twice
		if (std::string raw = j.dump(); Hash(raw) != Hash(this->m_tempArenaInfo.toStdString()))
		{
			const QString tempArenaInfo = QString::fromStdString(j.dump());
			this->m_tempArenaInfo = tempArenaInfo;
		}
		else
		{
			return;
		}

		emit this->StatusReady(Status::Loading, "Loading");
		LOG_TRACE("Opening websocket...");

		this->m_socket->open(QUrl(QString(g_wsAddress)));  // starts the request cycle
	}
}

// triggered when server response is received. processes the response, updates gui tables
void PotatoClient::OnResponse(const QString& message)
{
	LOG_TRACE("Closing websocket connection.");
	this->m_socket->close();

	LOG_TRACE("Received response from server: {}", message.toStdString());

	if (message.isNull() || message == "null")
	{
		emit this->StatusReady(Status::Error, "NULL Response");
		return;
	}

	LOG_TRACE("Parsing match.");
	const std::string raw = message.toUtf8().toStdString();
	auto res = StatsParser::ParseMatch(raw, true);

	if (!res.success)
	{
		emit this->StatusReady(Status::Error, "JSON Parse Error");
	}

	if (PotatoConfig().Get<bool>("match_history"))
	{
		if (Serializer::Instance().SaveMatch(res.match.info, this->m_tempArenaInfo.toStdString(), raw, res.csv.value()))
		{
			emit this->MatchHistoryChanged();
		}
	}

	LOG_TRACE("Updating tables.");
	emit this->MatchReady(res.match);

	emit this->StatusReady(Status::Ready, "Ready");
}

// checks the game path for a valid replays folder and triggers a new match run
FolderStatus PotatoClient::CheckPath()
{
	FolderStatus folderStatus;

	if (!this->m_watcher->directories().isEmpty())
		this->m_watcher->removePaths(this->m_watcher->directories());

	if (PotatoConfig().Get<bool>("override_replays_folder"))
	{
		this->m_watcher->addPath(QString::fromStdString(PotatoConfig().Get<std::string>("replays_folder")));
		folderStatus.statusText = "";
		this->TriggerRun();
		return folderStatus;
	}

	if (Game::CheckPath(PotatoConfig().Get<std::string>("game_folder"), folderStatus))
	{
		this->m_folderStatus = folderStatus;
		for (auto& folder : folderStatus.replaysPath)
		{
			if (!folder.empty())
			{
				this->m_watcher->addPath(QString::fromStdString(folder));
			}
		}
		this->TriggerRun();
	}
	return folderStatus;
}

// opens and reads a file
std::optional<std::string> PotatoClient::ReadArenaInfo(const std::string& filePath)
{
	LOG_TRACE("Reading arena info from: {}", filePath);

	using namespace std::chrono_literals;
	const auto startTime = std::chrono::high_resolution_clock::now();
	auto now = std::chrono::high_resolution_clock::now();

	const File file = File::Open(filePath, File::Flags::Open | File::Flags::Read);

	while (std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime) < 1000ms)
	{
		if (file.Size() > 0)
		{
			if (std::string arenaInfo; file.ReadString(arenaInfo))
			{
				return std::move(arenaInfo);
			}
			LOG_ERROR("Failed to read arena info: {}", File::LastError());
			return {};
		}
		std::this_thread::sleep_for(100ms);
		now = std::chrono::high_resolution_clock::now();
	}
	LOG_ERROR("Game failed to write arena info within 1 second.");
	return {};
}
