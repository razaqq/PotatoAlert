// Copyright 2020 <github.com/razaqq>

#include "Updater.hpp"
#include "Logger.hpp"
#include "Version.hpp"
#include <windows.h>
#include <string>
#include <sstream>
#include <filesystem>
#include <QtNetwork>
#include <QUrl>
#include <QFile>
#include <QApplication>
#include <QEventLoop>
#include <nlohmann/json.hpp>

#include <iostream>
#include <QDebug>


using nlohmann::json;
using PotatoAlert::Updater;
using PotatoAlert::Version;
namespace fs = std::filesystem;

// needs libssl-1_1-x64.dll and libcrypto-1_1-x64.dll from OpenSSL
// const char* updateURL = "https://ci.appveyor.com/api/projects/razaqq/PotatoAlert2/artifacts/build/PotatoAlert.zip";
const char* updateURL = "http://ipv4.download.thinkbroadband.com/5MB.zip";
const char* versionURL = "https://api.github.com/repos/razaqq/PotatoAlert/releases/latest";


// makes a request to the github api and checks if there is a new version available
bool Updater::updateAvailable()
{
    QEventLoop loop;
    auto manager = new QNetworkAccessManager();

    QNetworkReply* reply = manager->get(QNetworkRequest(QUrl(versionURL)));
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
	const fs::path tempFile = (root / "PotatoAlert_temp.exe");
	const fs::path newFile = (root / "PotatoAlert_new.exe");
	const fs::path oldFile = (root / "PotatoAlert.exe");

	// TODO: get updateURL dynamically from repo

	this->download(newFile.string());

	// TODO: maybe do some checksum check

	/*
    try
	{
		if (fs::exists(temp))
			fs::remove(temp);
    	if (fs::exists(dst))  // i mean this should always be true, but...
    		fs::rename(dst, temp);
    	if (Updater::download(dst))
			fs::copy(src, dst);
	}
    catch (fs::filesystem_error& e)
	{
		Logger::Error("Failed to update: {}", e.what());
	}
	*/

	/*
    // CreateProcess API initialization
    STARTUPINFO siStartupInfo;
    PROCESS_INFORMATION piProcessInfo;
    memset(&siStartupInfo, 0, sizeof(siStartupInfo));
    memset(&piProcessInfo, 0, sizeof(piProcessInfo));
    siStartupInfo.cb = sizeof(siStartupInfo);

    ::CreateProcess(buffer, // application name/path
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

    // this does not return.
    */
}

bool Updater::download(const std::string& targetFile)
{
	std::cout << "STARTING DOWNLOAD" << std::endl;

	auto manager = new QNetworkAccessManager();
	auto reply = manager->get(QNetworkRequest(QUrl(updateURL)));

	connect(reply, &QNetworkReply::downloadProgress, [this](qint64 bytesReceived, qint64 bytesTotal)
	{
		if (bytesTotal == 0)
			return;
		emit this->downloadProgress(bytesReceived, bytesTotal);
	});

	connect(reply, &QNetworkReply::finished, [this, reply, targetFile]()
	{

		if (reply->error() == QNetworkReply::NoError)
		{
		 	std::cout << "FINISHED, saving to: " << targetFile << std::endl;

			auto file = new QFile(QString::fromStdString(targetFile));
			if (file->open(QFile::WriteOnly))
			{
				file->write(reply->readAll());
				file->flush();
				file->close();
			}
			// TODO: Unzip and restart new binary
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
