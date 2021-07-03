// Copyright 2021 <github.com/razaqq>

#include "Updater.hpp"
#include "Log.hpp"
#include "Version.hpp"
#include "Json.hpp"
#include <chrono>
#include <filesystem>
#include <format>
#include <string>
#include <utility>
#include <QtNetwork>
#include <QUrl>
#include <QFile>
#include <QApplication>
#include <QEventLoop>
#include <QElapsedTimer>
#include <zip.h>
#include <Windows.h>  // TODO: use win32 instead
#include <shellapi.h>


using PotatoUpdater::Updater;
using PotatoAlert::Version;
namespace fs = std::filesystem;

// needs libssl-1_1-x64.dll and libcrypto-1_1-x64.dll from OpenSSL
static std::string_view g_updateURL = "https://github.com/razaqq/PotatoAlert/releases/latest/download/PotatoAlert.zip";
static std::string_view g_versionURL = "https://api.github.com/repos/razaqq/PotatoAlert/releases/latest";

struct DownloadProgress
{
	void Reset()
	{
		this->m_bytesSinceTick = Q_INT64_C(0);
		this->timer.restart();
	}

	void Update(qint64 bytesReceived)
	{
		if (!this->m_started)
		{
			this->m_started = true;
			this->timer.start();
		}

		this->m_bytesSinceTick += bytesReceived;

		if (this->timer.hasExpired(1e3))
		{
			this->m_speed = this->m_bytesSinceTick / 1e5f / this->timer.elapsed();  // MB/s

			// convert to kilobyte in case we are slow
			if (this->m_speed < 1.0f)
			{
				this->m_speed *= 1e3;
				this->m_unit = "kB/s";
			}
			else
			{
				this->m_unit = "MB/s";
			}

			this->Reset();
		}
	}

	[[nodiscard]] std::string ToString() const
	{
		return std::format("{:.1f} {}", this->m_speed, this->m_unit);
	}

private:
	QElapsedTimer timer;
	bool m_started = false;
	float m_speed = 0.0f;
	const char* m_unit = "MB/s";
	qint64 m_bytesSinceTick = Q_INT64_C(0);
} g_downloadProgress;

// makes a request to the github api and checks if there is a new version available
bool Updater::UpdateAvailable()
{
#ifndef NDEBUG
	return true;
#endif

	QEventLoop loop;
	auto manager = new QNetworkAccessManager();

	QNetworkRequest request;
	request.setUrl(QUrl(std::string(g_versionURL).c_str()));
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
	QNetworkReply* reply = manager->get(request);
	connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
	loop.exec();

	if (reply->error())
	{
		LOG_ERROR("Network reply for update failed: {}", reply->errorString().toStdString());
		return false;
	}

	json j;
	sax_no_exception sax(j);
	if (!json::sax_parse(reply->readAll().toStdString(), &sax))
	{
		LOG_ERROR("ParseError while parsing github api response as JSON.");
		return false;
	}

	return Version(j["tag_name"].get<std::string>()) > Version(QApplication::applicationVersion().toStdString());
}

// starts the update process
void Updater::Run()
{
	LOG_TRACE("Starting update...");

	if (!ElevationInfo().first)
	{
		LOG_ERROR("Updater needs to be started with admin privileges.");
		End(false);
	}

	LOG_TRACE("Starting download...");
	QNetworkReply* reply = this->Download();
	// TODO: get g_updateURL dynamically from repo
	// TODO: maybe do some checksum check on the downloaded archive

	connect(reply, &QNetworkReply::finished, [reply]()
	{
		// check if there was an error with the download
		if (reply->error() != QNetworkReply::NoError)
		{
			LOG_ERROR("Failed to download update: {}", reply->errorString().toStdString());
			End(false);
		}

		LOG_TRACE("Update downloaded successfully.");

		const std::string archive = UpdateArchive().string();
		const std::string dest = UpdateDest().string();

		if (auto file = new QFile(QString::fromStdString(archive)); file->open(QFile::WriteOnly))
		{
			file->write(reply->readAll());
			file->flush();
			file->close();
			LOG_TRACE("Update archive saved successfully in {}", Updater::UpdateArchive().string());
		}
		else
		{
			LOG_ERROR("Failed to Save update reply to file.");
			End(false);
		}

		// make backup
		if (!CreateBackup()) End(false);

		// rename exe/dll to trash
		if (!RenameToTrash())
		{
			LOG_ERROR("Failed to rename exe/dll to trash.");
			End(false, true);
		}

		// Unpack archive
		LOG_TRACE("Extracting archive to: {}", dest);
		if (!Unpack(archive.c_str(), dest.c_str()))
		{
			LOG_ERROR("Failed to Unpack archive.");
			End(false, true);
		}

		End(true);
	});
}

// unpacks an archive into a folder
bool Updater::Unpack(const char* file, const char* dest)
{
	LOG_INFO("Extracting zip archive: {}", file);

	static int i = 0;
	static const char* d = dest;
	auto on_extract_entry = [](const char* filename, void* arg)
	{
		int n = *static_cast<int*>(arg);
		LOG_INFO("Extracted: {} ({}/{})", fs::relative(filename, d).string(), ++i, n);

		return 0;
	};

	if (struct zip_t *zip = zip_open(file, 0, 'r'))
	{
		int n = zip_total_entries(zip);
		zip_close(zip);

		int status = zip_extract(file, dest, on_extract_entry, &n);
		return status == 0;
	}
	return false;
}

// downloads the update archive
QNetworkReply* Updater::Download()
{
	auto manager = new QNetworkAccessManager();

	QNetworkRequest request;
	request.setUrl(QUrl(std::string(g_updateURL).c_str()));
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
	auto reply = manager->get(request);

	connect(reply, &QNetworkReply::downloadProgress, [this](qint64 bytesReceived, qint64 bytesTotal)
	{
		if (bytesTotal == 0)
			return;

		g_downloadProgress.Update(bytesReceived);

		const QString progress = std::format("{:.1f}/{:.1f} MB", bytesReceived/1e6f, bytesTotal/1e6f).c_str();
		const QString speedStr = QString::fromStdString(g_downloadProgress.ToString());
		emit this->downloadProgress(static_cast<int>(bytesReceived * 100 / bytesTotal), progress, speedStr);
	});

	connect(reply, &QNetworkReply::sslErrors, [reply](const QList<QSslError>&)
	{
		LOG_ERROR(reply->errorString().toStdString());
		End(false);
	});

	connect(reply, &QNetworkReply::errorOccurred, [reply](QNetworkReply::NetworkError)
	{
		LOG_ERROR(reply->errorString().toStdString());
		End(false);
	});

	return reply;
}

// restarts the application
void Updater::End(bool success, bool revert)
{
	if (revert)
		RevertBackup();
	RemoveBackup();

	if (success)
		StartUpdater("--clear");
	else
		StartMain();
	ExitProcess(0); // exit this process
}

// gets info about the elevation state {bool isElevated, bool canElevate}
std::pair<bool, bool> Updater::ElevationInfo()
{
	HANDLE hToken;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
	{
		TOKEN_ELEVATION_TYPE tet;
		DWORD returnLength = 0;
		if (GetTokenInformation(hToken, TokenElevationType, &tet, sizeof(tet), &returnLength))
		{
			assert(returnLength == sizeof(tet));
			return {tet == TokenElevationTypeFull, tet == TokenElevationTypeLimited};
		}

	}
	CloseHandle(hToken);
	return {false, false};
}

bool Updater::CreateBackup()
{
	std::error_code ec;

	// check if old backup exists and remove it
	bool exists = fs::exists(BackupDest(), ec);
	if (ec)
	{
		LOG_ERROR("Failed to check if backup exists: {}", ec.message());
		return false;
	}

	if (exists)
	{
		fs::remove_all(BackupDest(), ec);
		if (ec)
		{
			LOG_ERROR("Failed to remove old backup: {}", ec.message());
			return false;
		}
	}

	// copy new backup
	fs::copy(UpdateDest(), BackupDest(), fs::copy_options::recursive, ec);
	if (ec)
	{
		LOG_ERROR("Failed to copy backup to backup dest: {}", ec.message());
		return false;
	}

	// fix permissions
	fs::permissions(BackupDest(), fs::perms::all, ec);
	if (ec)
	{
		LOG_ERROR("Failed to change permissions of backup: {}", ec.message());
		return false;
	}
	return true;
}

bool Updater::RemoveBackup()
{
	std::error_code ec;
	fs::remove_all(BackupDest(), ec);

	if (ec)
	{
		LOG_ERROR("Failed to delete backup: {}", ec.message());
		return false;
	}
	return true;
}

bool Updater::RevertBackup()
{
	std::error_code ec;
	fs::copy(BackupDest(),UpdateDest(),
			fs::copy_options::recursive | fs::copy_options::overwrite_existing,
			ec);

	if (ec)
	{
		LOG_ERROR("Failed to revert backup: {}", ec.message());
		return false;
	}
	return true;
}

// renames all exe/dll files to .trash
bool Updater::RenameToTrash()
{
	std::error_code ec;

	auto it = fs::recursive_directory_iterator(UpdateDest(), ec);
	if (ec)
	{
		LOG_ERROR("Failed to get directory iterator: {}", ec.message());
		return false;
	}

	for (auto& p : it)
	{
		const std::string ext = p.path().extension().string();

		bool regularFile = p.is_regular_file(ec);
		if (ec)
		{
			LOG_ERROR("Failed to check if {} is regular file: {}", p.path().string(), ec.message());
			return false;
		}

		if (regularFile && (ext == ".dll" || ext == ".exe"))
		{
			std::string newName = p.path().string() + ".trash";
			fs::rename(p, newName, ec);
			if (ec)
			{
				LOG_ERROR("Failed to rename {} to {}: {}", p.path().string(), newName, ec.message());
				return false;
			}
		}
	}
	return true;
}

// deletes all .trash files
void Updater::RemoveTrash()
{
	std::error_code ec;

	auto it = fs::recursive_directory_iterator(UpdateDest(), ec);
	if (ec)
	{
		LOG_ERROR("Failed to get directory iterator: {}", ec.message());
		return;
	}

	for (auto& p : it)
	{
		const std::string ext = p.path().extension().string();

		bool regularFile = p.is_regular_file(ec);
		if (ec)
			LOG_ERROR("Failed to check if {} is regular file: {}", p.path().string(), ec.message());

		if (regularFile && (ext == ".trash"))
		{
			fs::remove(p, ec);
			if (ec)
				LOG_ERROR("Failed to remove file {}: {}", p.path().string(), ec.message());
		}
	}
}

bool Updater::CreateNewProcess(std::string_view path, std::string_view args, bool elevated)
{
	const char* lpVerb = elevated ? "runas" : "open";
	const std::string pathStr = std::string(path);
	const std::string argsStr = std::string(args);
	SHELLEXECUTEINFOA sei = {
			sizeof(sei),
			SEE_MASK_NO_CONSOLE,
			nullptr,
			lpVerb,
			pathStr.c_str(),
			argsStr.c_str(),
			nullptr,
			SW_SHOWNORMAL
	};
	return ShellExecuteExA(&sei);
}
