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
#include "StatsParser.h"


// statuses
#define STATUS_READY 0
#define STATUS_LOADING 1
#define STATUS_ERROR 2

using PotatoAlert::PotatoClient;
using nlohmann::json;

const QUrl url("ws://www.perry-swift.de:33333");
// const QUrl url("ws://192.168.178.36:10000");
const std::string arenaInfoFile = "tempArenaInfo.json";

PotatoClient::PotatoClient(Config* config, Logger* logger) : QObject()
{
	this->config = config;
	this->logger = logger;
}

void PotatoClient::init()
{
	connect(this->config, &Config::modified, this, &PotatoClient::updateReplaysPath);

	connect(socket, &QWebSocket::connected, [this] {
		this->logger->Debug("Websocket open, sending arena info...");
		socket->sendTextMessage(this->tempArenaInfo);
	});

	connect(socket, &QWebSocket::textMessageReceived, this, &PotatoClient::onResponse);

	connect(this->watcher, &QFileSystemWatcher::directoryChanged, this, &PotatoClient::onDirectoryChanged);
	emit this->status(STATUS_READY, "Ready");
	
	this->updateReplaysPath();  // sets replays path and triggers initial run
}

// sets filesystem watcher to current replays folder, triggered when config is modified
void PotatoClient::updateReplaysPath()
{
	this->logger->Debug("Updating replays path.");

	if (this->watcher->directories().length() > 0)
		this->watcher->removePaths(this->watcher->directories());

	QString newPath = QString::fromStdString(config->j["/replays_folder"_json_pointer]);
	if (newPath != "")
	{
		this->watcher->addPath(newPath);  // watch new path
		this->onDirectoryChanged(newPath);  // trigger run
	}
}

// triggered whenever a file gets modified in the replays path
void PotatoClient::onDirectoryChanged(const QString& path)
{
	this->logger->Debug("Directory changed.");

	std::string filePath = path.toStdString() + "\\" + arenaInfoFile;
	std::string tempPath = std::filesystem::temp_directory_path().append(arenaInfoFile + ".temp").string();

	if (std::filesystem::exists(filePath))
	{
		try
		{
			// read arena info from file, copy file to avoid locking it
			std::string arenaInfo;
			try {
				if (!std::filesystem::copy_file(filePath, tempPath, std::filesystem::copy_options::overwrite_existing))
				{
					this->logger->Debug("failed to copy file");
					return;
				}

				std::ifstream fs(tempPath);
				std::stringstream buffer;
				buffer << fs.rdbuf();
				arenaInfo = buffer.str();
				fs.close();

				std::filesystem::remove(tempPath);
			}
			catch (std::filesystem::filesystem_error& e) {
				std::cerr << e.what() << std::endl;
				return;
			}

			std::string s = "ArenaInfo read from file. Content: " + arenaInfo;
			this->logger->Debug(s.c_str());

			// parse arenaInfo to json and add region
			nlohmann::json j = nlohmann::json::parse(arenaInfo);
			j["region"] = this->config->get<std::string>("region");
			QString newArenaInfo = QString::fromStdString(j.dump());

			// make sure we dont pull the same match twice
			if (newArenaInfo != this->tempArenaInfo)
				this->tempArenaInfo = newArenaInfo;
			else
				return;

			emit this->status(STATUS_LOADING, "Loading");
			
			this->logger->Debug("Opening websocket.");
			this->socket->open(url);  // starts the request cycle
			// TODO: handle connection error with timeout
		}
		catch (nlohmann::json::parse_error& e)
		{
			this->logger->Error("Failed to parse arenaInfoFile to json.");
			this->logger->Error(e.what());
			emit this->status(STATUS_ERROR, "JSON Parse Error");
		}
	}
}

// triggered when server response is received. processes the response, updates gui tables
void PotatoClient::onResponse(const QString& message)
{
	this->logger->Debug("Closing websocket connection.");
	this->socket->close();

	std::string d = "Received response from server: ";
	d += message.toStdString();
	this->logger->Debug(d.c_str());

	json j;
	try {
		j = json::parse(message.toStdString());
	}
	catch (json::parse_error& e) {
		this->logger->Error("ParseError while parsing server response json.");
		this->logger->Error(e.what());
		emit this->status(STATUS_ERROR, "JSON Parse Error");
		return;
	}

	this->logger->Debug("Updating tables.");

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
		this->logger->Error("ParseError while parsing server response json.");
		this->logger->Error(e.what());
		emit this->status(STATUS_ERROR, "JSON Parse Error");
		return;
	}
	catch (json::type_error& e) {
		this->logger->Error("TypeError while parsing server response json.");
		this->logger->Error(e.what());
		emit this->status(STATUS_ERROR, "JSON Type Error");
		return;
	}
}
