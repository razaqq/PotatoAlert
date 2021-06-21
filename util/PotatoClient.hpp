// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Game.hpp"
#include "StatsParser.hpp"
#include <QFileSystemWatcher>
#include <QLabel>
#include <QObject>
#include <QString>
#include <QTableWidgetItem>
#include <QWebSocket>
#include <array>
#include <optional>
#include <string>
#include <variant>
#include <vector>


using PotatoAlert::Game::FolderStatus;
using namespace PotatoAlert::StatsParser;

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
	void SetFolderStatus(const FolderStatus& status);
private:
	void OnResponse(const QString& message);
	void OnDirectoryChanged(const QString& path);
	void UpdateReplaysPath();
	static std::optional<std::string> ReadArenaInfo(const std::string& filePath);

	QWebSocket* socket = new QWebSocket();
	QFileSystemWatcher* watcher = new QFileSystemWatcher();

	QString tempArenaInfo;
	FolderStatus fStatus;
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
