// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Game.hpp"
#include "StatsParser.hpp"

#include <QFileSystemWatcher>
#include <QObject>
#include <QString>
#include <QTableWidgetItem>
#include <QtWebSockets/QWebSocket>

#include <optional>
#include <string>
#include <variant>


using PotatoAlert::Game::FolderStatus;

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
	PotatoClient(const PotatoClient&) = delete;
	PotatoClient(PotatoClient&&) noexcept = delete;
	PotatoClient& operator=(const PotatoClient&) = delete;
	PotatoClient& operator=(PotatoClient&&) noexcept = delete;
	~PotatoClient() override = default;

	static PotatoClient& Instance()
	{
		static PotatoClient pc;
		return pc;
	}
	void Init();
	void TriggerRun();
	FolderStatus CheckPath();
private:
	PotatoClient() = default;
	void OnResponse(const QString& message);
	void OnDirectoryChanged(const QString& path);
	static std::optional<std::string> ReadArenaInfo(const std::string& filePath);

	QWebSocket* m_socket = new QWebSocket();
	QFileSystemWatcher* m_watcher = new QFileSystemWatcher();

	QString m_tempArenaInfo;
	FolderStatus m_folderStatus;
signals:
#pragma clang diagnostic push
#pragma ide diagnostic ignored "NotImplementedFunctions"
	void MatchReady(const StatsParser::Match& match);
	void MatchHistoryChanged();
	void StatusReady(Status status, const std::string& statusText);
#pragma clang diagnostic pop
};

}  // namespace PotatoAlert
