// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QObject>
#include <QString>
#include <QFileSystemWatcher>
#include <QMainWindow>
#include <QtWebSockets>
#include <QLabel>
#include <QTableWidgetItem>
#include <string>
#include <vector>
#include <variant>
#include <nlohmann/json.hpp>
#include "Config.h"
#include "Logger.h"


typedef std::vector<std::vector<std::variant<QLabel*, QTableWidgetItem*>>> teamType;

namespace PotatoAlert {

class PotatoClient :  public QObject
{
	Q_OBJECT
public:
	PotatoClient(Config* config, Logger* l);
	void init();
private:
	void onResponse(const QString& message);
	void onDirectoryChanged(const QString& path);
	void updateReplaysPath();

	Config* config;
	Logger* logger;
	QWebSocket* socket = new QWebSocket();
	QFileSystemWatcher* watcher = new QFileSystemWatcher;

	QString tempArenaInfo;
signals:
	void teamsReady(std::vector<teamType> team1);
	void avgReady(std::vector<QString> avgs);
	void clansReady(std::vector<QString> clans);
	void wowsNumbersReady(std::vector<std::vector<QString>> wowsNumbers);
	void status(int statusID, const std::string& statusText);
};

};  // namespace PotatoAlert
