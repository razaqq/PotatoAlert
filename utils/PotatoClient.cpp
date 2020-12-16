// Copyright 2020 <github.com/razaqq>

#include "Config.hpp"
#include "Logger.hpp"
#include "Game.hpp"
#include "StatsParser.hpp"
#include "PotatoClient.hpp"
#include <QUrl>
#include <QDir>
#include <QObject>
#include <QLabel>
#include <QFileSystemWatcher>
#include <QFileInfo>
#include <QTextStream>
#include <QIODevice>
#include <QWebSocket>
#include <QSizePolicy>
#include <QTableWidgetItem>
#include <QString>
#include <QColor>
#include <QMetaObject>
#include <iostream>
#include <string>
#include <sstream>
#include <filesystem>
#include <fmt/format.h>


// statuses
#define STATUS_READY 0
#define STATUS_LOADING 1
#define STATUS_ERROR 2

using PotatoAlert::PotatoClient;
using nlohmann::json;
namespace fs = std::filesystem;

const char* wsAddr = "ws://www.perry-swift.de:33333";
// const char* wsAddr = "ws://192.168.178.36:10000";

void PotatoClient::init()
{
    emit this->status(STATUS_READY, "Ready");

    // handle connection
	connect(this->socket, &QWebSocket::connected, [this]
	{
        Logger::Debug("Websocket open, sending arena info...");
		this->socket->sendTextMessage(this->tempArenaInfo);
	});

	// handle error
    connect(this->socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), [=](QAbstractSocket::SocketError error)
    {
		switch (error)
		{
			case QAbstractSocket::ConnectionRefusedError:
				emit this->status(STATUS_ERROR, "Connection Refused");
				break;
			case QAbstractSocket::SocketTimeoutError:
				emit this->status(STATUS_ERROR, "Socket Timeout");
				break;
			case QAbstractSocket::HostNotFoundError:
				emit this->status(STATUS_ERROR, "Host Not Found");
				break;
			case QAbstractSocket::NetworkError:
				// TODO: this is a shit way of doing this, maybe create a Qt bug report?
				if (this->socket->errorString().contains("timed", Qt::CaseInsensitive))
					emit this->status(STATUS_ERROR, "Connection Timeout");
				else
					emit this->status(STATUS_ERROR, "Network Error");
				break;
			case QAbstractSocket::RemoteHostClosedError:
				emit this->status(STATUS_ERROR, "Host Closed Conn.");
				return;
			default:
				emit this->status(STATUS_ERROR, "Websocket Error");
				break;
		}
        Logger::Error(this->socket->errorString().toStdString());
    });

	connect(this->socket, &QWebSocket::textMessageReceived, this, &PotatoClient::onResponse);
	connect(this->watcher, &QFileSystemWatcher::directoryChanged, this, &PotatoClient::onDirectoryChanged);

	for (auto& path : this->watcher->directories())  // trigger run
        this->onDirectoryChanged(path);
}

// sets filesystem watcher to current replays folder, triggered when config is modified
void PotatoClient::updateReplaysPath()
{
    Logger::Debug("Updating replays path.");

	if (!this->watcher->directories().isEmpty())
		this->watcher->removePaths(this->watcher->directories());

	for (auto& folder : this->fStatus.replaysPath)
        if (!folder.empty())
            this->watcher->addPath(QString::fromStdString(folder));
}

// triggered whenever a file gets modified in a replays path
void PotatoClient::onDirectoryChanged(const QString& path)
{
    Logger::Debug("Directory changed.");

    std::string filePath = fmt::format("{}\\tempArenaInfo.json", path.toStdString());
	std::string tempPath = fs::temp_directory_path().append("tempArenaInfo.json.temp").string();

	if (fs::exists(filePath))
	{
		try
		{
			// read arena info from file, copy file to avoid locking it
			std::string arenaInfo;
			try
			{
				if (!fs::copy_file(filePath, tempPath, fs::copy_options::overwrite_existing))
				{
                    Logger::Debug("Failed to copy file");
					return;
				}

				std::ifstream fs(tempPath);
				std::stringstream buffer;
				buffer << fs.rdbuf();
				arenaInfo = buffer.str();
				fs.close();

				fs::remove(tempPath);
			}
			catch (fs::filesystem_error& e)
			{
				Logger::Error(e.what());
				return;
			}

            Logger::Debug("ArenaInfo read from file. Content: {}", arenaInfo);

			// parse arenaInfo to json and add region
			nlohmann::json j = nlohmann::json::parse(arenaInfo);
			j["region"] = this->fStatus.region;

			// set stats mode from config
			switch (PotatoConfig().get<int>("stats_mode"))
			{
				case current:
					if (j.contains("matchGroup"))
						j["statsMode"] = j["matchGroup"];
					else
						j["statsMode"] = "pvp";
					break;
				case pvp:
					j["statsMode"] = "pvp";
					break;
				case ranked:
					j["statsMode"] = "ranked";
					break;
				case clan:
					j["statsMode"] = "clan";
					break;
			}

			QString newArenaInfo = QString::fromStdString(j.dump());

			// make sure we dont pull the same match twice
			if (newArenaInfo != this->tempArenaInfo)
				this->tempArenaInfo = newArenaInfo;
			else
				return;

			emit this->status(STATUS_LOADING, "Loading");

            Logger::Debug("Opening websocket...");

			this->socket->open(QUrl(QString(wsAddr)));  // starts the request cycle
		}
		catch (nlohmann::json::parse_error& e)
		{
			Logger::Error("Failed to parse arena info file to JSON: {}", e.what());
			emit this->status(STATUS_ERROR, "JSON Parse Error");
		}
	}
}

// triggered when server response is received. processes the response, updates gui tables
void PotatoClient::onResponse(const QString& message)
{
    Logger::Debug("Closing websocket connection.");
	this->socket->close();

    Logger::Debug("Received response from server: {}", message.toStdString());

    if (message.isNull() || message == "null")
    {
		emit this->status(STATUS_ERROR, "NULL Response");
    	return;
    }

	try
	{
		json j;
		j = json::parse(message.toStdString());

		// save match to csv file
		if (PotatoConfig().get<bool>("save_csv"))
			this->csvWriter.saveMatch(message.toStdString());

		Logger::Debug("Updating tables.");

		auto matchGroup = j["matchGroup"].get<std::string>();
		auto team1Json = j["team1"].get<json>();
		auto team2Json = j["team2"].get<json>();

		auto team1 = StatsParser::parseTeam(team1Json, matchGroup);
		auto team2 = StatsParser::parseTeam(team2Json, matchGroup);
		emit this->teamsReady(std::vector{ team1, team2 });

		auto avg = StatsParser::parseAvg(team1Json);
		auto avg2 = StatsParser::parseAvg(team2Json);
		avg.insert(avg.end(), avg2.begin(), avg2.end());
		emit this->avgReady(avg);

		auto wowsNumbers1 = StatsParser::parseWowsNumbersProfile(team1Json);
		auto wowsNumbers2 = StatsParser::parseWowsNumbersProfile(team2Json);
		emit this->wowsNumbersReady(std::vector<std::vector<QString>>{ wowsNumbers1, wowsNumbers2 });

		if (matchGroup == "clan")
		{
			auto clans = StatsParser::parseClan(team1Json);
			auto clan2 = StatsParser::parseClan(team2Json);
			clans.insert(clans.end(), clan2.begin(), clan2.end());
			emit this->clansReady(clans);
		}
		else
		{
			emit this->clansReady(std::vector<QString>{});
		}

		emit this->status(STATUS_READY, "Ready");
	}
	catch (json::parse_error& e)
	{
		Logger::Error("ParseError while parsing server response JSON: {}", e.what());
		emit this->status(STATUS_ERROR, "JSON Parse Error");
		return;
	}
	catch (json::type_error& e)
	{
		Logger::Error("TypeError while parsing server response JSON: {}", e.what());
		emit this->status(STATUS_ERROR, "JSON Type Error");
		return;
	}
}

// sets the folder status and triggers a new match run
void PotatoClient::setFolderStatus(folderStatus& status)
{
    this->fStatus = status;
    this->updateReplaysPath();
}
