// Copyright 2021 <github.com/razaqq>

#include "Updater.hpp"
#include "Logger.hpp"
#include "Version.hpp"
#include <chrono>
#include <filesystem>
#include <format>
#include <string>
#include <sstream>
#include <chrono>
#include <utility>
#include <filesystem>
#include <QtNetwork>
#include <QUrl>
#include <QFile>
#include <QApplication>
#include <QEventLoop>
#include <QElapsedTimer>
#include <Windows.h>
#include <shellapi.h>
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <zip.h>


using nlohmann::json;
using PotatoUpdater::Updater;
using PotatoAlert::Version;
using PotatoAlert::Logger;
namespace fs = std::filesystem;

// needs libssl-1_1-x64.dll and libcrypto-1_1-x64.dll from OpenSSL
static std::string_view updateURL = "https://github.com/razaqq/PotatoAlert/releases/latest/download/PotatoAlert.zip";
static std::string_view versionURL = "https://api.github.com/repos/razaqq/PotatoAlert/releases/latest";

// makes a request to the github api and checks if there is a new version available
bool Updater::UpdateAvailable()
{
	QEventLoop loop;
	auto manager = new QNetworkAccessManager();

	QNetworkRequest request;
	request.setUrl(QUrl(std::string(versionURL).c_str()));
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
	QNetworkReply* reply = manager->get(request);
	connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
	loop.exec();

	if (reply->error())
	{
		Logger::Error("Network reply for update failed: {}", reply->errorString().toStdString());
		return false;
	}

	json j;
	try
	{
		j = json::parse(reply->readAll().toStdString());
	}
	catch (json::parse_error& e)
	{
		Logger::Error("Failed to parse github api response as JSON: {}", e.what());
		return false;
	}
	catch (json::type_error& e)
	{
		Logger::Error("{}", e.what());
		return false;
	}

	auto remoteVersion = j["tag_name"].get<std::string>();
	auto localVersion = QApplication::applicationVersion().toStdString();

#ifdef NDEBUG
	return Version(remoteVersion) > Version(localVersion);
#else
	return true;
#endif
}

// starts the update process
void Updater::Run()
{
	Logger::Debug("Starting update...");

	if (!Updater::ElevationInfo().first)
	{
		Logger::Error("Updater needs to be started with admin privileges.");
		Updater::End(false);
	}

	Logger::Debug("Starting download...");
	QNetworkReply* reply = this->Download();
	// TODO: get updateURL dynamically from repo
	// TODO: maybe do some checksum check on the downloaded archive

	connect(reply, &QNetworkReply::finished, [reply]()
	{
		// check if there was an error with the download
		if (reply->error() != QNetworkReply::NoError)
		{
			Logger::Error("Failed to download update: {}", reply->errorString().toStdString());
			Updater::End(false);
		}

		Logger::Debug("Update downloaded successfully.");

		const std::string archive = Updater::UpdateArchive().string();
		const std::string dest = Updater::UpdateDest().string();

		// Save reply to file
		auto file = new QFile(QString::fromStdString(archive));
		if (file->open(QFile::WriteOnly))
		{
			file->write(reply->readAll());
			file->flush();
			file->close();
			Logger::Debug("Update archive saved successfully in {}",
					Updater::UpdateArchive().string());
		}
		else
		{
			Logger::Error("Failed to Save update reply to file.");
			Updater::End(false);
		}

		// make backup
		if (!Updater::CreateBackup()) Updater::End(false);

		// rename exe/dll to trash
		if (!Updater::RenameToTrash())
		{
			Logger::Error("Failed to rename exe/dll to trash.");
			Updater::End(false, true);
		}

		// Unpack archive
		Logger::Debug("Extracting archive to: {}", dest);
		if (!Updater::Unpack(archive.c_str(), dest.c_str()))
		{
			Logger::Error("Failed to Unpack archive.");
			Updater::End(false, true);
		}

		Updater::End(true);
	});
}

// unpacks an archive into a folder
bool Updater::Unpack(const char* file, const char* dest)
{
	Logger::Info("Extracting zip archive: {}", file);

	static int i = 0;
	static const char* d = dest;
	auto on_extract_entry = [](const char* filename, void* arg)
	{
		int n = *static_cast<int*>(arg);
		Logger::Info("Extracted: {} ({}/{})", fs::relative(filename, d).string(), ++i, n);

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
	request.setUrl(QUrl(std::string(updateURL).c_str()));
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
	auto reply = manager->get(request);

	struct DownloadProgress
	{
		QElapsedTimer timer;
		bool started = false;
		float speed = 0.0f;
		const char* unit = "MB/s";
		qint64 bytesSinceTick = Q_INT64_C(0);

		void reset()
		{
			this->bytesSinceTick = Q_INT64_C(0);
			this->timer.restart();
		}

		void update(qint64 bytesReceived)
		{
			if (!this->started)
			{
				this->started = true;
				this->timer.start();
			}

			this->bytesSinceTick += bytesReceived;

			if (this->timer.hasExpired(1e3))
			{
				this->speed = this->bytesSinceTick / 1e5f / this->timer.elapsed();  // MB/s

				// convert to kilobyte in case we are slow
				if (this->speed < 1.0f)
				{
					this->speed *= 1e3;
					this->unit = "kB/s";
				}
				else
				{
					this->unit = "MB/s";
				}

				this->reset();
			}
		}
	};
	auto p = new DownloadProgress();
	// std::unique_ptr<DownloadProgress> p(new DownloadProgress());

	connect(reply, &QNetworkReply::downloadProgress, [this, p](qint64 bytesReceived, qint64 bytesTotal)
	{
		if (bytesTotal == 0)
			return;

		if (p)
			p->update(bytesReceived);

		QString progress = std::format("{:.1f}/{:.1f} MB", bytesReceived/1e6f, bytesTotal/1e6f).c_str();
		QString speedStr = std::format("{:.1f} {}", p->speed, p->unit).c_str();
		emit this->downloadProgress(static_cast<int>(bytesReceived * 100 / bytesTotal), progress, speedStr);
	});

	connect(reply, &QNetworkReply::finished, [p]()
	{
		delete p;
	});

	connect(reply, &QNetworkReply::sslErrors, [reply](const QList<QSslError>&)
	{
		Logger::Error(reply->errorString().toStdString());
		Updater::End(false);
	});

	connect(reply, &QNetworkReply::errorOccurred, [reply](QNetworkReply::NetworkError)
	{
		Logger::Error(reply->errorString().toStdString());
		Updater::End(false);
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
	bool exists = fs::exists(Updater::BackupDest(), ec);
	if (ec)
	{
		Logger::Error("Failed to check if backup exists: {}", ec.message());
		return false;
	}

	if (exists)
	{
		fs::remove_all(Updater::BackupDest(), ec);
		if (ec)
		{
			Logger::Error("Failed to remove old backup: {}", ec.message());
			return false;
		}
	}

	// copy new backup
	fs::copy(Updater::UpdateDest(), Updater::BackupDest(), fs::copy_options::recursive, ec);
	if (ec)
	{
		Logger::Error("Failed to copy backup to backup dest: {}", ec.message());
		return false;
	}

	// fix permissions
	fs::permissions(Updater::BackupDest(), fs::perms::all, ec);
	if (ec)
	{
		Logger::Error("Failed to change permissions of backup: {}", ec.message());
		return false;
	}
	return true;
}

bool Updater::RemoveBackup()
{
	std::error_code ec;
	fs::remove_all(Updater::BackupDest(), ec);

	if (ec)
	{
		Logger::Error("Failed to delete backup: {}", ec.message());
		return false;
	}
	return true;
}

bool Updater::RevertBackup()
{
	std::error_code ec;
	fs::copy(Updater::BackupDest(),Updater::UpdateDest(),
			fs::copy_options::recursive | fs::copy_options::overwrite_existing,
			ec);

	if (ec)
	{
		Logger::Error("Failed to revert backup: {}", ec.message());
		return false;
	}
	return true;
}

// renames all exe/dll files to .trash
bool Updater::RenameToTrash()
{
	std::error_code ec;

	auto it = fs::recursive_directory_iterator(Updater::UpdateDest(), ec);
	if (ec)
	{
		Logger::Error("Failed to get directory iterator: {}", ec.message());
		return false;
	}

	for (auto& p : it)
	{
		const std::string ext = p.path().extension().string();

		bool regularFile = p.is_regular_file(ec);
		if (ec)
		{
			Logger::Error("Failed to check if {} is regular file: {}", p.path().string(), ec.message());
			return false;
		}

		if (regularFile && (ext == ".dll" || ext == ".exe"))
		{
			std::string newName = p.path().string() + ".trash";
			fs::rename(p, newName, ec);
			if (ec)
			{
				Logger::Error("Failed to rename {} to {}: {}", p.path().string(), newName, ec.message());
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

	auto it = fs::recursive_directory_iterator(Updater::UpdateDest(), ec);
	if (ec)
	{
		Logger::Error("Failed to get directory iterator: {}", ec.message());
		return;
	}

	for (auto& p : it)
	{
		const std::string ext = p.path().extension().string();

		bool regularFile = p.is_regular_file(ec);
		if (ec)
			Logger::Error("Failed to check if {} is regular file: {}", p.path().string(), ec.message());

		if (regularFile && (ext == ".trash"))
		{
			fs::remove(p, ec);
			if (ec)
				Logger::Error("Failed to remove file {}: {}", p.path().string(), ec.message());
		}
	}
}

bool Updater::CreateNewProcess(std::string_view path, std::string_view args, bool elevated)
{
	const char* lpVerb = elevated ? "runas" : "open";
	std::string pathStr = std::string(path);
	std::string argsStr = std::string(args);
	SHELLEXECUTEINFO sei = {
			sizeof(sei),
			SEE_MASK_NO_CONSOLE,
			nullptr,
			lpVerb,
			pathStr.c_str(),
			argsStr.c_str(),
			nullptr,
			SW_SHOWNORMAL
	};
	return ShellExecuteEx(&sei);
}
