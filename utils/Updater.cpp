// Copyright 2020 <github.com/razaqq>

#include "Updater.hpp"
#include "Logger.hpp"
#include "Version.hpp"
#include <windows.h>
#include <string>
#include <sstream>
#include <chrono>
#include <filesystem>
#include <QtNetwork>
#include <QUrl>
#include <QFile>
#include <QApplication>
#include <QEventLoop>
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <zip.h>


using nlohmann::json;
using PotatoAlert::Updater;
using PotatoAlert::Version;
namespace fs = std::filesystem;

// needs libssl-1_1-x64.dll and libcrypto-1_1-x64.dll from OpenSSL
const char* updateURL = "https://ci.appveyor.com/api/projects/razaqq/potatoalert2/artifacts/build/PotatoAlert.zip";
const char* versionURL = "https://api.github.com/repos/razaqq/PotatoAlert/releases/latest";


// makes a request to the github api and checks if there is a new version available
bool Updater::updateAvailable()
{
    QEventLoop loop;
    auto manager = new QNetworkAccessManager();

    QNetworkRequest request;
    request.setUrl(QUrl(versionURL));
	request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    QNetworkReply* reply = manager->get(request);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error())
    {
        Logger::Debug(reply->errorString().toStdString());
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

    // return Version(remoteVersion) > Version(localVersion);
	return true;
}

void Updater::start()
{
    Logger::Debug("Starting update...");

    const fs::path root = fs::absolute(fs::current_path());
	const fs::path zipFile = (root / "PotatoAlert.zip");
	const fs::path dest = (root / "new");
	const fs::path exeFile = (root / "PotatoAlert.exe").string();  // TODO: get this dynamically, otherwise renaming can cause it to fail

	Updater::restart(exeFile.string().c_str(), "");

    try
	{
		if (fs::exists(zipFile))
			fs::remove(zipFile);

		if (!fs::exists(exeFile))  // TODO
			return;

		// TODO: get updateURL dynamically from repo

		this->download(zipFile.string(), dest.string(), exeFile.string());

		// TODO: maybe do some checksum check

		/*
    	if (fs::exists(dst))  // i mean this should always be true, but...
    		fs::rename(dst, temp);
    	if (Updater::download(dst))
			fs::copy(src, dst);
		 */
	}
    catch (fs::filesystem_error& e)
	{
		Logger::Error("Failed to update: {}", e.what());
	}
}

bool Updater::unpack(const char* file, const char* dest)
{
	Logger::Info("Extracting zip archive...");

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

bool Updater::download(const std::string& targetFile, const std::string& dest, const std::string& exeFile)
{
	auto manager = new QNetworkAccessManager();

	QNetworkRequest request;
	request.setUrl(QUrl(updateURL));
	request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
	auto reply = manager->get(request);

	typedef std::chrono::high_resolution_clock clock;
	clock::time_point startTime = clock::now();

	connect(reply, &QNetworkReply::downloadProgress, [this, startTime](qint64 bytesReceived, qint64 bytesTotal)
	{
		if (bytesTotal == 0)
			return;

		float timeDelta = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - startTime).count() / 1e3f;
		if (timeDelta == 0)  // make sure we dont get zero division
			timeDelta = 1;
		float downloadSpeed = bytesReceived / 1e6f / timeDelta;

		QString progress = fmt::format("{:.1f}/{:.1f} MB", bytesReceived/1e6f, bytesTotal/1e6f).c_str();
		QString speed = fmt::format("{:.1f} MB/s", downloadSpeed).c_str();
		emit this->downloadProgress(static_cast<int>(bytesReceived * 100 / bytesTotal), progress, speed);
	});

	connect(reply, &QNetworkReply::finished, [reply, targetFile, dest, exeFile]()
	{
		if (reply->error() == QNetworkReply::NoError)
		{
		 	Logger::Debug("FINISHED, saving to: {}", targetFile);

			auto file = new QFile(QString::fromStdString(targetFile));
			if (file->open(QFile::WriteOnly))
			{
				file->write(reply->readAll());
				file->flush();
				file->close();
			}
			// TODO: Unzip and restart new binary
			if (Updater::unpack(targetFile.c_str(), dest.c_str()))
				Updater::restart(exeFile.c_str(), "--changelog");
		}
	});

	/*
	connect(reply, &QNetworkReply::sslErrors, [this, reply](const QList<QSslError>& errors)
	{
		QString str = reply->errorString();
		emit this->errorOccurred(str);
	});
	 */

	connect(reply, &QNetworkReply::errorOccurred, [this, reply](QNetworkReply::NetworkError error)
	{
		QString str = reply->errorString();
		emit this->errorOccurred(str);
	});

    return true;
}

void Updater::restart(const char* exeFile, const char* args)
{
	STARTUPINFO siStartupInfo;
	PROCESS_INFORMATION piProcessInfo;
	memset(&siStartupInfo, 0, sizeof(siStartupInfo));
	memset(&piProcessInfo, 0, sizeof(piProcessInfo));
	siStartupInfo.cb = sizeof(siStartupInfo);

	::CreateProcess(exeFile, // application name/path
					nullptr, // command line (optional)
					nullptr, // no process attributes (default)
					nullptr, // default security attributes
					false,
					CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_CONSOLE,
					nullptr, // default env
					nullptr, // default working dir
					&siStartupInfo,
					&piProcessInfo);


	::TerminateProcess( GetCurrentProcess(),0);
	::ExitProcess(0); // exit this process
}