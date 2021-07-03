// Copyright 2020 <github.com/razaqq>

#include "PotatoClient.hpp"
#include "Config.hpp"
#include "Game.hpp"
#include "Json.hpp"
#include "Log.hpp"
#include "StatsParser.hpp"
#include <QColor>
#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QIODevice>
#include <QLabel>
#include <QObject>
#include <QSizePolicy>
#include <QString>
#include <QTableWidgetItem>
#include <QTextStream>
#include <QUrl>
#include <QWebSocket>
#include <chrono>
#include <filesystem>
#include <format>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <thread>


using PotatoAlert::PotatoClient;
namespace fs = std::filesystem;

const char* wsAddr = "ws://www.perry-swift.de:33333";
// const char* wsAddr = "ws://192.168.178.36:10000";

void PotatoClient::Init()
{
	emit this->status(Status::Ready, "Ready");

	// handle connection
	connect(this->m_socket, &QWebSocket::connected, [this]
	{
		LOG_TRACE("Websocket open, sending arena info...");
		this->m_socket->sendTextMessage(this->m_tempArenaInfo);
	});

	// handle error
	connect(this->m_socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), [=](QAbstractSocket::SocketError error)
	{
		switch (error)
		{
			case QAbstractSocket::ConnectionRefusedError:
				emit this->status(Status::Error, "Connection Refused");
				break;
			case QAbstractSocket::SocketTimeoutError:
				emit this->status(Status::Error, "Socket Timeout");
				break;
			case QAbstractSocket::HostNotFoundError:
				emit this->status(Status::Error, "Host Not Found");
				break;
			case QAbstractSocket::NetworkError:
				// TODO: this is a shit way of doing this, maybe create a Qt bug report?
				if (this->m_socket->errorString().contains("timed", Qt::CaseInsensitive))
					emit this->status(Status::Error, "Connection Timeout");
				else
					emit this->status(Status::Error, "Network Error");
				break;
			case QAbstractSocket::RemoteHostClosedError:
				emit this->status(Status::Error, "Host Closed Conn.");
				return;
			default:
				emit this->status(Status::Error, "Websocket Error");
				break;
		}
		LOG_ERROR(this->m_socket->errorString().toStdString());
	});

	connect(this->m_socket, &QWebSocket::textMessageReceived, this, &PotatoClient::OnResponse);
	connect(this->m_watcher, &QFileSystemWatcher::directoryChanged, this, &PotatoClient::OnDirectoryChanged);

	for (auto& path : this->m_watcher->directories())  // trigger run
		this->OnDirectoryChanged(path);
}

// sets filesystem watcher to current replays folder, triggered when config is modified
void PotatoClient::UpdateReplaysPath()
{
	LOG_TRACE("Updating replays path.");

	if (!this->m_watcher->directories().isEmpty())
		this->m_watcher->removePaths(this->m_watcher->directories());

	if (PotatoConfig().Get<bool>("override_replays_folder"))
	{
		this->m_watcher->addPath(QString::fromStdString(this->m_folderStatus.overrideReplaysPath));
	}
	else
	{
		for (auto& folder : this->m_folderStatus.replaysPath)
			if (!folder.empty())
				this->m_watcher->addPath(QString::fromStdString(folder));
	}
}

// triggered whenever a file gets modified in a replays path
void PotatoClient::OnDirectoryChanged(const QString& path)
{
	LOG_TRACE("Directory changed.");

	std::string filePath = std::format("{}\\tempArenaInfo.json", path.toStdString());

	if (fs::exists(filePath))
	{
		// read arena info from file
		auto arenaInfo = PotatoClient::ReadArenaInfo(filePath);
		if (!arenaInfo.has_value())
		{
			LOG_ERROR("Failed to read arena info from file.");
			emit this->status(Status::Error, "Reading ArenaInfo");
			return;
		}

		json j;
		sax_no_exception sax(j);
		if (!json::sax_parse(arenaInfo.value(), &sax))
		{
			LOG_ERROR("Failed to Parse arena info file as JSON.");
			emit this->status(Status::Error, "JSON Parse Error");
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

		QString tempArenaInfo = QString::fromStdString(j.dump());

		// make sure we dont pull the same match twice
		if (tempArenaInfo != this->m_tempArenaInfo)
			this->m_tempArenaInfo = tempArenaInfo;
		else
			return;

		emit this->status(Status::Loading, "Loading");
		LOG_TRACE("Opening websocket...");

		this->m_socket->open(QUrl(QString(wsAddr)));  // starts the request cycle
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
		emit this->status(Status::Error, "NULL Response");
		return;
	}

	json j;
	sax_no_exception sax(j);
	if (!json::sax_parse(message.toStdString(), &sax))
	{
		LOG_ERROR("ParseError while parsing server response JSON.");
		emit this->status(Status::Error, "JSON Parse Error");
		return;
	}

	LOG_TRACE("Parsing match.");
	StatsParser::Match match;
	ParseMatch(message.toUtf8().toStdString(), match, PotatoConfig().Get<bool>("save_csv"));

	LOG_TRACE("Updating tables.");
	emit this->matchReady(match);

	emit this->status(Status::Ready, "Ready");
}

// sets the folder status and triggers a new match run
void PotatoClient::SetFolderStatus(const FolderStatus& status)
{
	this->m_folderStatus = status;
	this->UpdateReplaysPath();
}

// opens and reads a file
std::optional<std::string> PotatoClient::ReadArenaInfo(const std::string& filePath)
{
	Logger::Debug("Reading arena info from: {}", filePath);

	using namespace std::chrono_literals;
	auto startTime = std::chrono::high_resolution_clock::now();
	auto now = std::chrono::high_resolution_clock::now();

	File file = File::Open(filePath, File::Flags::Open | File::Flags::Read);
	std::string arenaInfo;

	while (std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime) < 1000ms)
	{
		if (file.Size() > 0)
		{
			if (file.Read(arenaInfo))
			{
				return arenaInfo;
			}
			else
			{
				LOG_ERROR("Failed to read arena info: {}", File::LastError());
				return {};
			}
		}
		std::this_thread::sleep_for(100ms);
		now = std::chrono::high_resolution_clock::now();
	}
	LOG_ERROR("Game failed to write arena info within 1 second.");
	return {};
}
