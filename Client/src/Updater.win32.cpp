// Copyright 2021 <github.com/razaqq>

#include "Client/Updater.hpp"

#include "Core/Defer.hpp"
#include "Core/Directory.hpp"
#include "Core/Encoding.hpp"
#include "Core/Format.hpp"
#include "Core/Json.hpp"
#include "Core/Log.hpp"
#include "Core/Process.hpp"
#include "Core/String.hpp"
#include "Core/Version.hpp"
#include "Core/Zip.hpp"


#include <QApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QUrl>
#include <QtNetwork>

#include <chrono>
#include <filesystem>
#include <string>
#include <utility>

#include "win32.h"
#include "tlhelp32.h"

/*
 * CURRENT WAY:
 * Shut down main binary, start updater as admin
 * Download archive to temp
 * Create backup
 * Rename current files to .trash
 * Unpack archive into target
 * Restart main binary
 * Remove trash
 */

/*
 * MAYBE BETTER:
 * Shut down main binary, start updater as admin
 * Download archive to temp
 * Create backup
 * Rename current files to .trash
 * Unpack archive into temp
 * MoveFile() to target location, on the same drive this is atomic
 * Restart main binary
 */


using PotatoAlert::Client::Updater;
using namespace PotatoAlert::Core;
namespace fs = std::filesystem;

// needs libssl-1_1-x64.dll and libcrypto-1_1-x64.dll from OpenSSL
static constexpr std::string_view g_updateURL = "https://github.com/razaqq/PotatoAlert/releases/latest/download/{}";
static constexpr std::string_view g_versionURL = "https://api.github.com/repos/razaqq/PotatoAlert/releases/latest";
// TODO: beta https://api.github.com/repos/razaqq/PotatoAlert/releases


namespace {

std::optional<DWORD> FindProcessByName(LPCTSTR Name)
{
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 pe;
		ZeroMemory(&pe, sizeof(PROCESSENTRY32));
		pe.dwSize = sizeof(PROCESSENTRY32);
		Process32First(hSnap, &pe);
		do
		{
			if (!lstrcmpi(pe.szExeFile, Name))
			{
				return pe.th32ProcessID;
			}
		} while (Process32Next(hSnap, &pe));
	}
	return {};
}

struct DownloadProgress
{
	void Reset()
	{
		m_bytesSinceTick = Q_INT64_C(0);
		timer.restart();
	}

	void Update(qint64 bytesReceived)
	{
		if (!m_started)
		{
			m_started = true;
			timer.start();
		}

		m_bytesSinceTick += bytesReceived;

		if (timer.hasExpired(1e3))
		{
			m_speed = m_bytesSinceTick / 1e5f / timer.elapsed();  // MB/s

			// convert to kilobyte in case we are slow
			if (m_speed < 1.0f)
			{
				m_speed *= 1e3;
				m_unit = "kB/s";
			}
			else
			{
				m_unit = "MB/s";
			}

			Reset();
		}
	}

	[[nodiscard]] std::string ToString() const
	{
		return fmt::format("{:.1f} {}", m_speed, m_unit);
	}

private:
	QElapsedTimer timer;
	bool m_started = false;
	float m_speed = 0.0f;
	const char* m_unit = "MB/s";
	qint64 m_bytesSinceTick = Q_INT64_C(0);
} g_downloadProgress;

}  // namespace

// makes a request to the github api and checks if there is a new version available
bool Updater::UpdateAvailable()
{
#ifndef NDEBUG
	return false;
#endif

	QEventLoop loop;
	QNetworkAccessManager* manager = new QNetworkAccessManager();

	QNetworkRequest request;
	request.setUrl(QUrl(std::string(g_versionURL).c_str()));
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
	QNetworkReply* reply = manager->get(request);
	connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
	loop.exec();

	if (reply->error() != QNetworkReply::NoError)
	{
		LOG_ERROR("Network reply for update failed: {}", reply->errorString().toStdString());
		return false;
	}

	const auto tagName = Json::GetPointer<std::string, "/tag_name">(reply->readAll().toStdString());
	if (!tagName)
	{
		LOG_ERROR("Failed to get 'tag_name' from Github API response JSON.");
		return false;
	}

	return Version(*tagName) > Version(QApplication::applicationVersion().toStdString());
}

// starts the update process
void Updater::Run()
{
	LOG_INFO("Starting update...");

	if (!GetElevationInfo().IsElevated)
	{
		LOG_ERROR("Updater needs to be started with admin privileges.");
		End(false);
	}

	LOG_INFO("Starting download...");
	QNetworkReply* reply = Download();
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

		LOG_INFO("Update download successful.");

		const Path archive = UpdateArchive();
		const Path dest = UpdateDest();

		if (QFile* file = new QFile(archive); file->open(QFile::WriteOnly))
		{
			file->write(reply->readAll());
			file->flush();
			file->close();
			LOG_INFO(STR("Update archive saved successfully in {}"), Updater::UpdateArchive());
		}
		else
		{
			LOG_ERROR("Failed to Save update reply to file.");
			End(false);
		}

		// make backup
		if (!CreateBackup())
			End(false);

		// rename exe/dll to trash
		if (!RenameToTrash())
		{
			LOG_ERROR("Failed to rename exe/dll to trash.");
			End(false, true);
		}

		// Unpack archive
		LOG_INFO(STR("Extracting archive {} to {}"), archive, dest);

		const Zip zip = Zip::Open(archive, 0);
		if (!zip)
		{
			LOG_ERROR("Failed to open zip file: ");
			End(false, true);
		}

		int totalEntries = zip.EntryCount();
		int i = 0;
		auto onExtract = [dest, &i, totalEntries](const char* fileName) -> int
		{
			LOG_INFO(STR("Extracted: {} ({}/{})"), fs::relative(fileName, dest), ++i, totalEntries);
			return 0;
		};

		if (!Zip::Extract(archive, dest, onExtract))
		{
			LOG_ERROR("Failed to Unpack archive.");
			End(false, true);
		}

		LOG_INFO("Update complete!");
		End(true);
	});
}

// downloads the update archive
QNetworkReply* Updater::Download()
{
	QNetworkAccessManager* manager = new QNetworkAccessManager();

	QNetworkRequest request;
	request.setUrl(QUrl(fmt::format(g_updateURL, UpdateArchiveFile(CurrentEdition)).c_str()));
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
	auto reply = manager->get(request);

	connect(reply, &QNetworkReply::downloadProgress, [this](qint64 bytesReceived, qint64 bytesTotal)
	{
		if (bytesTotal == 0)
			return;

		g_downloadProgress.Update(bytesReceived);

		const QString progress = fmt::format("{:.1f}/{:.1f} MB", bytesReceived / 1e6f, bytesTotal / 1e6f).c_str();
		const QString speedStr = QString::fromStdString(g_downloadProgress.ToString());
		emit DownloadProgress(static_cast<int>(bytesReceived * 100 / bytesTotal), progress, speedStr);
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
	{
		StartUpdater("--clear");
	}
	else
	{
		StartMain();
	}
	QApplication::exit(0); // exit this process
	ExitCurrentProcess(0);
}

// gets info about the elevation state {bool isElevated, bool canElevate}
Updater::ElevationInfo Updater::GetElevationInfo()
{
	HANDLE hToken;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
	{
		PA_DEFER
		{
			CloseHandle(hToken);
		};
		TOKEN_ELEVATION_TYPE tet;
		DWORD returnLength = 0;
		if (GetTokenInformation(hToken, TokenElevationType, &tet, sizeof(tet), &returnLength))
		{
			assert(returnLength == sizeof(tet));
			return {tet == TokenElevationTypeFull, tet == TokenElevationTypeLimited};
		}
	}
	return {false, false};
}

bool Updater::CreateBackup()
{
	std::error_code ec;

	// check if old backup exists and remove it
	const bool exists = fs::exists(BackupDest(), ec);
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
		Path path = p.path();

		const bool regularFile = p.is_regular_file(ec);
		if (ec)
		{
			LOG_ERROR("Failed to check if {} is regular file: {}", path, ec);
			return false;
		}

		const std::string ext = path.extension().string();
		if (regularFile && (ext == ".dll" || ext == ".exe"))
		{
			path += Path(".trash");
			fs::rename(p, path, ec);
			if (ec)
			{
				LOG_ERROR("Failed to rename {} to .trash: {}", path, ec);
				return false;
			}
		}
	}
	return true;
}

// deletes all .trash files
void Updater::RemoveTrash()
{
	LOG_INFO("Clearing trash of old version.");

	// have to wait, otherwise some dlls are still locked by the other process
	WaitForOtherProcessExit();

	std::error_code ec;
	auto it = fs::recursive_directory_iterator(UpdateDest(), ec);
	if (ec)
	{
		LOG_ERROR("Failed to get directory iterator: {}", ec);
		return;
	}

	for (auto& p : it)
	{
		const bool regularFile = p.is_regular_file(ec);
		if (ec)
			LOG_ERROR("Failed to check if {} is regular file: {}", p.path(), ec);

		if (regularFile && p.path().extension().string() == ".trash")
		{
			fs::remove(p, ec);
			if (ec)
				LOG_ERROR("Failed to remove file {}: {}", p.path(), ec);
		}
	}
}

bool Updater::StartUpdater(std::string_view args)
{
	LOG_INFO("Restarting updater binary.");
	return CreateNewProcess(m_updaterBinary, args, true);
}

bool Updater::StartMain(std::string_view args)
{
	LOG_INFO("Restarting main binary.");
	return CreateNewProcess(m_mainBinary, args, false);
}

void Updater::WaitForOtherProcessExit()
{
	std::optional<DWORD> pid = FindProcessByName(TEXT("PotatoUpdater.exe"));

	if (pid && GetCurrentProcessId() != pid.value())
	{
		if (const HANDLE hHandle = OpenProcess(SYNCHRONIZE, false, pid.value()))
		{
			LOG_INFO("Waiting for other updater process to terminate");
			WaitForSingleObject(hHandle, 10000);
			LOG_INFO("Other updater process terminated");
		}
	}
}
