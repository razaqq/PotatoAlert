// Copyright 2020 <github.com/razaqq>

#include "Updater.hpp"
#include "Logger.h"
#include "Version.hpp"
#include <windows.h>
#include <string>
#include <sstream>
#include <filesystem>
#include <QtNetwork>
#include <QUrl>
#include <QApplication>
#include <QEventLoop>
#include <nlohmann/json.hpp>

#include <iostream>


using nlohmann::json;
using PotatoAlert::Updater;
using PotatoAlert::Version;
namespace fs = std::filesystem;

// needs libssl-1_1-x64.dll and libcrypto-1_1-x64.dll from OpenSSL
const char* updateURL = "";
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
        PotatoLogger().Error("Failed to parse github api response as JSON: {}", e.what());
        return false;
    }
    catch (json::type_error& e)
	{
		PotatoLogger().Error("{}", e.what());
		return false;
	}

    auto remoteVersion = j["tag_name"].get<std::string>();
    auto localVersion = QApplication::applicationVersion().toStdString();

    // return Version(remoteVersion) > Version(localVersion);
	return true;
}

void Updater::update()
{
    Logger::Debug("Starting update...");

    const fs::path root = fs::absolute(fs::current_path());
	const fs::path temp = (root / "PotatoAlert_temp.exe");
	const fs::path src = (root / "PotatoAlert_new.exe");
	const fs::path dst = (root / "PotatoAlert.exe");

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
    	PotatoLogger().Error("Failed to update: {}", e.what());
	}

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

bool Updater::download(const fs::path& targetFile)
{
    // connect(reply, &QNetworkReply::downloadProgress, progressDialog, &ProgressDialog::networkReplyProgress);
    return true;
}
