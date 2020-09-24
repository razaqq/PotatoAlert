// Copyright 2020 <github.com/razaqq>

#include "PotatoClient.h"
#include <QUrl>
#include <QDir>
#include <QObject>
#include <QLabel>
#include <QFileSystemWatcher>
#include <QFileInfo>
#include <QTextStream>
#include <QIODevice>
#include <QtWebSockets>
#include <QSizePolicy>
#include <QTableWidgetItem>
#include <QString>
#include <QColor>
#include <iostream>
#include <string>
#include <sstream>
#include <filesystem>
#include <fmt/format.h>
#include "Config.h"
#include "Logger.h"
#include "Game.h"
#include "StatsParser.h"

// statuses
#define STATUS_READY 0
#define STATUS_LOADING 1
#define STATUS_ERROR 2

using PotatoAlert::PotatoClient;
using nlohmann::json;
namespace fs = std::filesystem;

const QUrl url("ws://www.perry-swift.de:33333");
// const QUrl url("ws://192.168.178.36:10000");
const std::string arenaInfoFile = "tempArenaInfo.json";

PotatoClient::PotatoClient() : QObject()
{
}

void PotatoClient::init()
{
    // handle connection
	connect(this->socket, &QWebSocket::connected, [this] {
        Logger::Debug("Websocket open, sending arena info...");
		this->socket->sendTextMessage(this->tempArenaInfo);
	});

	// handle error
    connect(this->socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), [=](QAbstractSocket::SocketError error)
    {
        if (error == QAbstractSocket::ConnectionRefusedError)
            emit this->status(STATUS_ERROR, "Connection");
        else
            emit this->status(STATUS_ERROR, "Websocket Error");
        PotatoLogger().Error(this->socket->errorString().toStdString().c_str());
    });

	connect(this->socket, &QWebSocket::textMessageReceived, this, &PotatoClient::onResponse);

	connect(this->watcher, &QFileSystemWatcher::directoryChanged, this, &PotatoClient::onDirectoryChanged);
	emit this->status(STATUS_READY, "Ready");
}

// sets filesystem watcher to current replays folder, triggered when config is modified
void PotatoClient::updateReplaysPath()
{
    Logger::Debug("Updating replays path.");

	if (this->watcher->directories().length() > 0)
		this->watcher->removePaths(this->watcher->directories());

	for (auto& folder : this->fStatus.replaysPath)
    {
        QString newPath = QString::fromStdString(folder);
        if (newPath != "") {
            this->watcher->addPath(newPath);  // watch new path
            this->onDirectoryChanged(newPath);  // trigger run
        }
    }
}

// triggered whenever a file gets modified in a replays path
void PotatoClient::onDirectoryChanged(const QString& path)
{
    Logger::Debug("Directory changed.");

	std::string filePath = path.toStdString() + "\\" + arenaInfoFile;
	std::string tempPath = fs::temp_directory_path().append(arenaInfoFile + ".temp").string();

	if (fs::exists(filePath))
	{
		try
		{
			// read arena info from file, copy file to avoid locking it
			std::string arenaInfo;
			try {
				if (!fs::copy_file(filePath, tempPath, fs::copy_options::overwrite_existing))
				{
                    Logger::Debug("failed to copy file");
					return;
				}

				std::ifstream fs(tempPath);
				std::stringstream buffer;
				buffer << fs.rdbuf();
				arenaInfo = buffer.str();
				fs.close();

				fs::remove(tempPath);
			}
			catch (fs::filesystem_error& e) {
				std::cerr << e.what() << std::endl;
				return;
			}

			std::string s = "ArenaInfo read from file. Content: " + arenaInfo;
            Logger::Debug(s.c_str());

			// parse arenaInfo to json and add region
			nlohmann::json j = nlohmann::json::parse(arenaInfo);
			j["region"] = PotatoConfig().get<std::string>("region");
			QString newArenaInfo = QString::fromStdString(j.dump());

			// make sure we dont pull the same match twice
			if (newArenaInfo != this->tempArenaInfo)
				this->tempArenaInfo = newArenaInfo;
			else
				return;

			emit this->status(STATUS_LOADING, "Loading");

            Logger::Debug("Opening websocket.");
			this->socket->open(url);  // starts the request cycle
		}
		catch (nlohmann::json::parse_error& e)
		{
            PotatoLogger().Error("Failed to parse arenaInfoFile to json.");
            PotatoLogger().Error(e.what());
			emit this->status(STATUS_ERROR, "JSON Parse Error");
		}
	}
}

// triggered when server response is received. processes the response, updates gui tables
void PotatoClient::onResponse(const QString& message)
{
    Logger::Debug("Closing websocket connection.");
	this->socket->close();

	std::string d = "Received response from server: ";
	d += message.toStdString();
    Logger::Debug(d.c_str());

	json j;
	try {
		j = json::parse(message.toStdString());
	}
	catch (json::parse_error& e) {
        PotatoLogger().Error("ParseError while parsing server response json.");
        PotatoLogger().Error(e.what());
		emit this->status(STATUS_ERROR, "JSON Parse Error");
		return;
	}

	// save match to csv file
	if (PotatoConfig().get<bool>("save_csv"))
	    this->csvWriter.saveMatch(message.toStdString());

    Logger::Debug("Updating tables.");

	auto matchGroup = j["MatchGroup"].get<std::string>();
	auto team1Json = j["Team1"].get<json>();
	auto team2Json = j["Team2"].get<json>();

	try {
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
	catch (json::parse_error& e) {
        PotatoLogger().Error("ParseError while parsing server response json.");
        PotatoLogger().Error(e.what());
		emit this->status(STATUS_ERROR, "JSON Parse Error");
		return;
	}
	catch (json::type_error& e) {
        PotatoLogger().Error("TypeError while parsing server response json.");
        PotatoLogger().Error(e.what());
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
