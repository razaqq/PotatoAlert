// Copyright 2021 <github.com/razaqq>
#pragma once

#include <QtNetwork/QNetworkAccessManager>
#include <QWidget>

#include <filesystem>
#include <string>
#include <utility>


namespace PotatoAlert::Updater {

enum class Edition
{
	Windows,
	Linux
};

#if defined(WIN32)
	static constexpr Edition CurrentEdition = Edition::Windows;
#else
	static constexpr Edition CurrentEdition = Edition::Linux;
#endif

static constexpr std::string_view UpdateArchiveFile(Edition edition)
{
	switch (edition)
	{
		case Edition::Windows:
			return "PotatoAlert.zip";
		case Edition::Linux:
			return "PotatoAlert_linux.zip";
	}
	return "";
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
	using Path = std::filesystem::path;

	QNetworkReply* Download();

	[[noreturn]] static void End(bool success = true, bool revert = false);

	// functions to handle backup
	static bool CreateBackup();
	static bool RemoveBackup();
	static bool RevertBackup();

	// functions for executable/library renaming
	static bool RenameToTrash();

	// functions for paths
	static Path UpdateDest() { return std::filesystem::absolute(std::filesystem::current_path()); }
	static Path BackupDest() { return Path(std::filesystem::temp_directory_path() / "PotatoAlertBackup"); }
	static Path UpdateArchive() { return Path(std::filesystem::temp_directory_path() / UpdateArchiveFile(CurrentEdition)); }

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
