// Copyright 2020 <github.com/razaqq>
#pragma once

#include "CSVWriter.hpp"
#include "Game.hpp"
#include <QFileSystemWatcher>
#include <QLabel>
#include <QObject>
#include <QString>
#include <QTableWidgetItem>
#include <QWebSocket>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

using PotatoAlert::Game::folderStatus;

typedef std::vector<std::vector<std::variant<QLabel *, QTableWidgetItem *>>> teamType;

namespace PotatoAlert {

enum Status
{
	Ready,
	Loading,
	Error
};

class PotatoClient : public QObject
{
	Q_OBJECT
public:
	PotatoClient() = default;
	PotatoClient(const PotatoClient&) = delete;
	void init();
	void setFolderStatus(folderStatus& status);
private:
	void onResponse(const QString& message);
	void onDirectoryChanged(const QString& path);
	void updateReplaysPath();
	static std::tuple<bool, std::string> readArenaInfo(const std::string& filePath);

	QWebSocket* socket = new QWebSocket();
	QFileSystemWatcher* watcher = new QFileSystemWatcher();

	QString tempArenaInfo;
	folderStatus fStatus;
	CSVWriter csvWriter;
signals:
#pragma clang diagnostic push
#pragma ide diagnostic ignored "NotImplementedFunctions"
	void teamsReady(const std::vector<teamType>& team1);
	void avgReady(const std::vector<QString>& avgs);
	void clansReady(const std::vector<QString>& clans);
	void wowsNumbersReady(const std::vector<std::vector<QString>>& wowsNumbers);
	void status(PotatoAlert::Status status, const std::string& statusText);
#pragma clang diagnostic pop
};

}  // namespace PotatoAlert
