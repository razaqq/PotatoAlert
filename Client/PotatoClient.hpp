// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Core/Singleton.hpp"
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


using PotatoAlert::Client::Game::DirectoryStatus;

namespace PotatoAlert::Client {

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
	~PotatoClient() override = default;

	PA_SINGLETON(PotatoClient);

	void Init();
	void TriggerRun();
	DirectoryStatus CheckPath();

private:
	PotatoClient() = default;
	void OnResponse(const QString& message);
	void OnDirectoryChanged(const QString& path);

	QWebSocket m_socket;
	QFileSystemWatcher m_watcher;

	QString m_tempArenaInfo;
	DirectoryStatus m_dirStatus;

signals:
#pragma clang diagnostic push
#pragma ide diagnostic ignored "NotImplementedFunctions"
	void MatchReady(const StatsParser::Match& match);
	void MatchHistoryChanged();
	void StatusReady(Status status, const std::string& statusText);
#pragma clang diagnostic pop
};

}  // namespace PotatoAlert::Client
