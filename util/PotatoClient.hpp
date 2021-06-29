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

enum class Status
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
	PotatoClient& operator=(const PotatoClient&) = delete;
	/*
	PotatoClient(PotatoClient&& other) noexcept
		: socket(other.socket), watcher(other.watcher),
		  tempArenaInfo(std::move(other.tempArenaInfo)),
		  fStatus(std::move(other.fStatus))
	{
		Logger::Debug("MOVED");
	}
	*/
	void Init();
	void SetFolderStatus(const FolderStatus& status);
private:
	void OnResponse(const QString& message);
	void OnDirectoryChanged(const QString& path);
	void UpdateReplaysPath();
	static std::optional<std::string> ReadArenaInfo(const std::string& filePath);

	QWebSocket* m_socket = new QWebSocket();
	QFileSystemWatcher* m_watcher = new QFileSystemWatcher();

	QString m_tempArenaInfo;
	FolderStatus m_folderStatus;
signals:
#pragma clang diagnostic push
#pragma ide diagnostic ignored "NotImplementedFunctions"
	void matchReady(const Match& match);
	void status(PotatoAlert::Status status, const std::string& statusText);
#pragma clang diagnostic pop
};

}  // namespace PotatoAlert
