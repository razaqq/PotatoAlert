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
	static bool UpdateAvailable();

	void Run();

	// functions to start updater/main binary
	static inline bool StartUpdater(std::string_view args = "")
	{
		return CreateNewProcess(m_updaterBinary, args, true);
	}
	static inline bool StartMain(std::string_view args = "")
	{
		return CreateNewProcess(m_mainBinary, args, false);
	}

	static void RemoveTrash();
private:
	QNetworkReply* Download();

	static void End(bool success = true, bool revert = false);

	// functions to handle backup
	static bool CreateBackup();
	static bool RemoveBackup();
	static bool RevertBackup();

	// functions for executable/library renaming
	static bool RenameToTrash();

	// functions for paths
	static inline fs::path UpdateDest() { return fs::absolute(fs::current_path()); };
	static inline fs::path BackupDest() { return fs::path(fs::temp_directory_path() / "PotatoAlertBackup"); };
	static inline fs::path UpdateArchive() { return fs::path(fs::temp_directory_path() / "PotatoAlert.zip"); };

	static bool CreateNewProcess(std::string_view path, std::string_view args, bool elevated);

	static bool Unpack(const char* file, const char* dest);
	static std::pair<bool, bool> ElevationInfo();

	constexpr static std::string_view m_updaterBinary = "PotatoUpdater.exe";
	constexpr static std::string_view m_mainBinary = "PotatoAlert.exe";
signals:
#pragma clang diagnostic push
#pragma ide diagnostic ignored "NotImplementedFunctions"
	void downloadProgress(int percent, const QString& progress, const QString& speed);
#pragma clang diagnostic pop
};

}  // namespace PotatoUpdater
