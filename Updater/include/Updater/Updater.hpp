// Copyright 2021 <github.com/razaqq>
#pragma once

#include <QtNetwork/QNetworkAccessManager>
#include <QWidget>

#include <filesystem>
#include <string>
#include <utility>


namespace fs = std::filesystem;

namespace PotatoAlert::Updater {

enum class Edition
{
	Qt6_Windows10,
	Qt5_Windows7,
	Linux
};

#if WIN32 && QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	static constexpr Edition CurrentEdition = Edition::Qt6_Windows10;
#elif WIN32
	static constexpr Edition CurrentEdition = Edition::Qt5_Windows7;
#else
	static constexpr Edition CurrentEdition = Edition::Linux;
#endif

static constexpr std::string_view UpdateArchiveFile(Edition edition)
{
	switch (edition)
	{
		case Edition::Qt6_Windows10:
			return "PotatoAlert.zip";
		case Edition::Qt5_Windows7:
			return "PotatoAlert_win7.zip";
		case Edition::Linux:
			return "PotatoAlert_linux.zip";
	}
}

class Updater : public QWidget
{
	Q_OBJECT

public:
	static bool UpdateAvailable();

	void Run();

	// functions to start updater/main binary
	static bool StartUpdater(std::string_view args = "");
	static bool StartMain(std::string_view args = "");

	static void RemoveTrash();

private:
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
	static fs::path UpdateArchive() { return fs::path(fs::temp_directory_path() / UpdateArchiveFile(CurrentEdition)); }

	struct ElevationInfo
	{
		bool IsElevated;
		bool CanElevate;
	};
	static ElevationInfo GetElevationInfo();

	static void WaitForOtherProcessExit();

	constexpr static std::string_view m_updaterBinary = "PotatoUpdater.exe";
	constexpr static std::string_view m_mainBinary = "PotatoAlert.exe";

signals:
	void DownloadProgress(int percent, const QString& progress, const QString& speed);
};

}  // namespace PotatoAlert::Updater
