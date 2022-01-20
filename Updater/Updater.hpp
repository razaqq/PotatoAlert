// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Core/Singleton.hpp"

#include <QtNetwork/QNetworkAccessManager>
#include <QWidget>
// #include <QtNetwork>

#include <filesystem>
#include <string_view>


namespace fs = std::filesystem;

namespace PotatoUpdater {

class Updater : public QWidget
{
	Q_OBJECT
public:
	PA_SINGLETON(Updater);

	static bool UpdateAvailable();

	void Run();

	// functions to start updater/main binary
	static bool StartUpdater(std::string_view args = "");
	static bool StartMain(std::string_view args = "");

	static void RemoveTrash();
private:
	Updater() = default;
	QNetworkReply* Download();

	[[noreturn]] static void End(bool success = true, bool revert = false);

	// functions to handle backup
	static bool CreateBackup();
	static bool RemoveBackup();
	static bool RevertBackup();

	// functions for executable/library renaming
	static bool RenameToTrash();

	// functions for paths
	static fs::path UpdateDest() { return fs::absolute(fs::current_path()); }
	static fs::path BackupDest() { return fs::path(fs::temp_directory_path() / "PotatoAlertBackup"); }
	static fs::path UpdateArchive() { return fs::path(fs::temp_directory_path() / "PotatoAlert.zip"); }

	static std::pair<bool, bool> ElevationInfo();

	constexpr static std::string_view m_updaterBinary = "PotatoUpdater.exe";
	constexpr static std::string_view m_mainBinary = "PotatoAlert.exe";
signals:
#pragma clang diagnostic push
#pragma ide diagnostic ignored "NotImplementedFunctions"
	void DownloadProgress(int percent, const QString& progress, const QString& speed);
#pragma clang diagnostic pop
};

}  // namespace PotatoUpdater
