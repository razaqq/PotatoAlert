// Copyright 2021 <github.com/razaqq>
#pragma once

#include <string>
#include <filesystem>
#include <QObject>
#include <QWidget>
#include <QtNetwork>
#include <QNetworkAccessManager>


namespace fs = std::filesystem;

namespace PotatoUpdater {

class Updater : public QWidget
{
	Q_OBJECT
public:
	// functions called from PotatoAlert
	static bool updateAvailable();

	void run();
	static void end(bool success = true, bool revert = false);

	static bool unpack(const char* file, const char* dest);
	static std::pair<bool, bool> elevationType();

	// functions for paths
	static inline fs::path updateDest() { return fs::absolute(fs::current_path()); };
	static inline fs::path backupDest() { return fs::path(fs::temp_directory_path() / "PotatoAlertBackup"); };
	static inline fs::path updateArchive() { return fs::path(fs::temp_directory_path() / "PotatoAlert.zip"); };

	// functions to handle backup
	static bool createBackup();
	static bool removeBackup();
	static bool revertBackup();

	// functions for executable/library renaming
	static bool renameToTrash();
	static void removeTrash();

	static bool createProcess(std::string_view path = updaterBinary, std::string_view args = "", bool elevated = true);
	constexpr static std::string_view updaterBinary = "PotatoUpdater.exe";
	constexpr static std::string_view mainBinary = "PotatoAlert.exe";
private:
	QNetworkReply* download();
signals:
#pragma clang diagnostic push
#pragma ide diagnostic ignored "NotImplementedFunctions"
	void downloadProgress(int percent, const QString& progress, const QString& speed);
#pragma clang diagnostic pop
};

}  // namespace PotatoUpdater
