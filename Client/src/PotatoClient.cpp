// Copyright 2020 <github.com/razaqq>

#include "Client/Config.hpp"
#include "Client/Game.hpp"
#include "Client/ReplayAnalyzer.hpp"
#include "Client/MatchHistory.hpp"
#include "Client/PotatoClient.hpp"
#include "Client/StatsParser.hpp"

#include "Core/Defer.hpp"
#include "Core/Json.hpp"
#include "Core/Log.hpp"
#include "Core/Sha256.hpp"

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


using PotatoAlert::Client::PotatoClient;
using namespace PotatoAlert::Core;

static const char* g_wsAddress = "ws://www.perry-swift.de:33333";
// static const char* g_wsAddress = "ws://192.168.178.36:10000";

namespace {

struct TempArenaInfoResult
{
	bool exists = true;
	bool error = false;
	std::string data;
};

// opens and reads a file
static TempArenaInfoResult ReadArenaInfo(std::string_view filePath)
{
	LOG_TRACE("Reading arena info from: {}", filePath);

	TempArenaInfoResult result;

	using namespace std::chrono_literals;
	const auto startTime = std::chrono::high_resolution_clock::now();
	auto now = std::chrono::high_resolution_clock::now();

	File file = File::Open(filePath, File::Flags::Open | File::Flags::Read);

	if (!file)
	{
		LOG_ERROR("Failed to open arena info file: {}", File::LastError());
		return { File::Exists(filePath), true, "" };
	}

	// close the file when the scope ends
	auto defer = MakeDefer([&file]() { if (file) file.Close(); });

	while (std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime) < 1000ms)
	{
		if (!File::Exists(filePath))
		{
			// we have to assume its not an error, because the game sometimes touches it at the end of a match
			// when in reality its about to get deleted
			return { false, false, "" };
		}

		// the game has written to the file
		if (file.Size() > 0)
		{
			std::string arenaInfo;
			if (file.ReadString(arenaInfo))
			{
				return { true, false, arenaInfo };
			}
			LOG_ERROR("Failed to read arena info file: {}", File::LastError());
			return { true, true, "" };
		}
		std::this_thread::sleep_for(100ms);
		now = std::chrono::high_resolution_clock::now();
	}
	LOG_ERROR("Game failed to write arena info within 1 second.");
	return { File::Exists(filePath), true, "" };
}

}

void PotatoClient::Init()
{
	emit StatusReady(Status::Ready, "Ready");

	// handle connection
	connect(&m_socket, &QWebSocket::connected, [this]
	{
		LOG_TRACE("Websocket open, sending arena info...");
		m_socket.sendTextMessage(QString::fromStdString(m_tempArenaInfo));
	});

	// handle error
	connect(&m_socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), [this](QAbstractSocket::SocketError error)
	{
		switch (error)
		{
			case QAbstractSocket::ConnectionRefusedError:
				emit StatusReady(Status::Error, "Connection Refused");
				break;
			case QAbstractSocket::SocketTimeoutError:
				emit StatusReady(Status::Error, "Socket Timeout");
				break;
			case QAbstractSocket::HostNotFoundError:
				emit StatusReady(Status::Error, "Host Not Found");
				break;
			case QAbstractSocket::NetworkError:
				// TODO: this is a shit way of doing this, maybe create a Qt bug report?
				if (m_socket.errorString().contains("timed", Qt::CaseInsensitive))
					emit StatusReady(Status::Error, "Connection Timeout");
				else
					emit StatusReady(Status::Error, "Network Error");
				break;
			case QAbstractSocket::RemoteHostClosedError:
				emit StatusReady(Status::Error, "Host Closed Conn.");
				return;
			default:
				emit StatusReady(Status::Error, "Websocket Error");
				break;
		}
		LOG_ERROR(m_socket.errorString().toStdString());
	});

	connect(&m_socket, &QWebSocket::textMessageReceived, this, &PotatoClient::OnResponse);

	connect(&m_watcher, &Core::DirectoryWatcher::FileChanged, this, &PotatoClient::OnFileChanged);
	connect(&m_watcher, &Core::DirectoryWatcher::FileChanged, &m_replayAnalyzer, &ReplayAnalyzer::OnFileChanged);

	connect(&m_replayAnalyzer, &ReplayAnalyzer::ReplaySummaryReady, this, &PotatoClient::MatchSummaryChanged);

	TriggerRun();
}

// triggers a run with the current replays folders
void PotatoClient::TriggerRun()
{
	m_watcher.ForceDirectoryChanged();
}

// triggered whenever a file gets modified in a replays path
void PotatoClient::OnFileChanged(const std::string& file)
{
	LOG_TRACE("Directory changed.");

	if (fs::path(file).filename() != "tempArenaInfo.json" || !File::Exists(file))
	{
		return;
	}

	// read arena info from file
	TempArenaInfoResult arenaInfo = ReadArenaInfo(file);
	if (arenaInfo.error)
	{
		LOG_ERROR("Failed to read arena info from file.");
		emit StatusReady(Status::Error, "Reading ArenaInfo");
		return;
	}

	if (!arenaInfo.exists)
	{
		LOG_TRACE("Arena info did not exist when directory changed.");
		return;
	}

	if (arenaInfo.data.empty())
	{
		LOG_TRACE("tempArenaInfo.json was empty.");
		return;
	}

	json j;
	sax_no_exception sax(j);
	if (!json::sax_parse(arenaInfo.data, &sax))
	{
		LOG_ERROR("Failed to Parse arena info file as JSON.");
		emit StatusReady(Status::Error, "JSON Parse Error");
		return;
	}

	LOG_TRACE("ArenaInfo read from file. Content: {}", arenaInfo.data);

	// make sure we don't pull the same match twice
	std::string hash;
	Core::Sha256(arenaInfo.data, hash);
	if (hash == m_lastArenaInfoHash)
		return;
	m_lastArenaInfoHash = hash;

	// add region
	j["region"] = m_dirStatus.region;

	// set stats mode from config
	switch (PotatoConfig().Get<ConfigKey::StatsMode>())
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

	m_tempArenaInfo = j.dump();

	emit StatusReady(Status::Loading, "Loading");
	LOG_TRACE("Opening websocket...");

	m_socket.open(QUrl(QString(g_wsAddress)));  // starts the request cycle
}

// triggered when server response is received. processes the response, updates gui tables
void PotatoClient::OnResponse(const QString& message)
{
	LOG_TRACE("Closing websocket connection.");
	m_socket.close();

	LOG_TRACE("Received response from server: {}", message.toStdString());

	if (message.isNull() || message == "null")
	{
		emit StatusReady(Status::Error, "NULL Response");
		return;
	}

	LOG_TRACE("Parsing match.");
	const std::string raw = message.toUtf8().toStdString();
	const StatsParser::StatsParseResult res = StatsParser::ParseMatch(raw, true);

	if (!res.success)
	{
		emit StatusReady(Status::Error, "JSON Parse Error");
	}

	if (PotatoConfig().Get<ConfigKey::MatchHistory>())
	{
		if (MatchHistory::Instance().SaveMatch(res.match.info, m_tempArenaInfo, m_lastArenaInfoHash, raw, res.csv.value()))
		{
			emit MatchHistoryChanged();
		}
	}

	LOG_TRACE("Updating tables.");
	emit MatchReady(res.match);

	emit StatusReady(Status::Ready, "Ready");
}

// checks the game path for a valid replays folder and triggers a new match run
DirectoryStatus PotatoClient::CheckPath()
{
	DirectoryStatus dirStatus;

	m_watcher.ClearDirectories();

	const std::string gamePath = PotatoConfig().Get<ConfigKey::GameDirectory>();

	if (PotatoConfig().Get<ConfigKey::OverrideReplaysDirectory>())
	{
		const std::string replaysPath = PotatoConfig().Get<ConfigKey::ReplaysDirectory>();
		m_watcher.WatchDirectory(replaysPath);
		dirStatus.statusText = "";

		m_replayAnalyzer.AnalyzeDirectory(gamePath);
		TriggerRun();

		return dirStatus;
	}

	if (Game::CheckPath(gamePath, dirStatus))
	{
		m_dirStatus = dirStatus;

		// start watching all replays paths
		for (auto& folder : dirStatus.replaysPath)
		{
			if (!folder.empty())
			{
				m_watcher.WatchDirectory(folder);
			}
		}

		// lets check the entire game folder, replays might be hiding everywhere
		m_replayAnalyzer.AnalyzeDirectory(gamePath);

		TriggerRun();
	}
	return dirStatus;
}
