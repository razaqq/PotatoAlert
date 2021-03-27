// Copyright 2021 <github.com/razaqq>

#include "Updater.hpp"
#include "Logger.hpp"
#include "Version.hpp"
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


#include <QtDebug>

using nlohmann::json;
using PotatoUpdater::Updater;
using PotatoAlert::Version;
using PotatoAlert::Logger;
namespace fs = std::filesystem;

// needs libssl-1_1-x64.dll and libcrypto-1_1-x64.dll from OpenSSL
static std::string_view updateURL = "https://ci.appveyor.com/api/projects/razaqq/potatoalert2/artifacts/build/PotatoAlert.zip";
static std::string_view versionURL = "https://api.github.com/repos/razaqq/PotatoAlert/releases/latest";

// makes a request to the github api and checks if there is a new version available
bool Updater::updateAvailable()
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
void Updater::run()
{
	Logger::Debug("Starting update...");

	if (!Updater::elevationType().first)
	{
		Logger::Error("Updater needs to be started with admin privileges.");
		Updater::end(false);
	}

	Logger::Debug("Starting download...");
	QNetworkReply* reply = this->download();
	// TODO: get updateURL dynamically from repo
	// TODO: maybe do some checksum check on the downloaded archive

	connect(reply, &QNetworkReply::finished, [this, reply]()
	{
		// check if there was an error with the download
		if (reply->error() != QNetworkReply::NoError)
		{
			Logger::Error("Failed to download update: {}", reply->errorString().toStdString());
			Updater::end(false);
		}

		Logger::Debug("Update downloaded successfully.");

		const std::string archive = Updater::updateArchive().string();
		const std::string dest = Updater::updateDest().string();

		// save reply to file
		auto file = new QFile(QString::fromStdString(archive));
		if (file->open(QFile::WriteOnly))
		{
			file->write(reply->readAll());
			file->flush();
			file->close();
			Logger::Debug("Update archive saved successfully in {}", Updater::updateArchive().string());
		}
		else
		{
			Logger::Error("Failed to save update reply to file.");
			Updater::end(false);
		}

		// make backup
		if (!Updater::createBackup())
			Updater::end(false);

		// rename exe/dll to trash
		if (!Updater::renameToTrash())
			Updater::end(false, true);

		// unpack archive
		Logger::Debug("Extracting archive to: {}", dest);
		if (!Updater::unpack(archive.c_str(), dest.c_str()))
		{
			Logger::Error("Failed to unpack archive.");
			Updater::end(false, true);
		}

		Updater::end(true);
	});
}

// unpacks an archive into a folder
bool Updater::unpack(const char* file, const char* dest)
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
QNetworkReply* Updater::download()
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

		QString progress = fmt::format("{:.1f}/{:.1f} MB", bytesReceived/1e6f, bytesTotal/1e6f).c_str();
		QString speedStr = fmt::format("{:.1f} {}", p->speed, p->unit).c_str();
		emit this->downloadProgress(static_cast<int>(bytesReceived * 100 / bytesTotal), progress, speedStr);
	});

	connect(reply, &QNetworkReply::finished, [p]()
	{
		delete p;
	});

	connect(reply, &QNetworkReply::sslErrors, [reply](const QList<QSslError>& errors)
	{
		Logger::Error(reply->errorString().toStdString());
		Updater::end(false);
	});

	connect(reply, &QNetworkReply::errorOccurred, [reply](QNetworkReply::NetworkError error)
	{
		Logger::Error(reply->errorString().toStdString());
		Updater::end(false);
	});

	return reply;
}

// restarts the application
void Updater::end(bool success, bool revert)
{
	if (revert)
		Updater::revertBackup();
	Updater::removeBackup();

	if (success)
		Updater::createProcess(updaterBinary, "--clear");
	else
		Updater::createProcess(mainBinary);
	ExitProcess(0); // exit this process
}

// gets info about the elevation state {bool isElevated, bool canElevate}
std::pair<bool, bool> Updater::elevationType()
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

bool Updater::createBackup()
{
	try
	{
		if (fs::exists(Updater::backupDest()))
			fs::remove_all(Updater::backupDest());
		fs::copy(Updater::updateDest(), Updater::backupDest(), fs::copy_options::recursive);
		fs::permissions(Updater::backupDest(), fs::perms::all);
		return true;
	}
	catch (fs::filesystem_error& e)
	{
		Logger::Error("Failed to create backup: {}", e.what());
		return false;
	}
}

bool Updater::removeBackup()
{
	try
	{
		fs::remove_all(Updater::backupDest());
		return true;
	}
	catch (fs::filesystem_error& e)
	{
		Logger::Error("Failed to delete backup: {}", e.what());
		return false;
	}
}

bool Updater::revertBackup()
{
	try
	{
		fs::copy(Updater::backupDest(), Updater::updateDest(), fs::copy_options::recursive | fs::copy_options::overwrite_existing);
		return true;
	}
	catch (fs::filesystem_error& e)
	{
		Logger::Error("Failed to revert backup: {}", e.what());
		return false;
	}
}

// renames all exe/dll files to .trash
bool Updater::renameToTrash()
{
	try
	{
		for (auto& p : fs::recursive_directory_iterator(Updater::updateDest()))
		{
			const std::string ext = p.path().extension().string();
			if (p.is_regular_file() && (ext == ".dll" || ext == ".exe"))
				fs::rename(p, p.path().string() + ".trash");
		}
		return true;
	}
	catch (fs::filesystem_error& e)
	{
		Logger::Error("Failed to rename exe/dll to trash: {}", e.what());
		return false;
	}
}

// deletes all .trash files
void Updater::removeTrash()
{
	for (auto& p : fs::recursive_directory_iterator(Updater::updateDest()))
	{
		try
		{
			const std::string ext = p.path().extension().string();
			if (p.is_regular_file() && (ext == ".trash"))
				fs::remove(p);
		}
		catch (fs::filesystem_error& e)
		{
			Logger::Error("Failed to remove trash: {}", e.what());
		}
	}
}

bool Updater::createProcess(std::string_view path, std::string_view args, bool elevated)
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
