// Copyright 2020 <github.com/razaqq>

#include "Updater.h"
#include "Logger.h"
#include "Version.h"
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


bool Updater::updateAvailable()
{
    QEventLoop loop;
    auto manager = new QNetworkAccessManager;

    QNetworkReply* reply = manager->get(QNetworkRequest(QUrl(versionURL)));
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error())
    {
        Logger::Debug(reply->errorString().toStdString().c_str());
        return false;
    }

    json j;
    try {
        j = json::parse(reply->readAll().toStdString());
    }
    catch (json::parse_error& e) {
        PotatoLogger().Error("Failed to parse github api response as json.");
        return false;
    }

    auto remoteVersion = j["tag_name"].get<std::string>();
    auto localVersion = QApplication::applicationVersion().toStdString();

    return Version(remoteVersion) > Version(localVersion);
}

void Updater::update()
{
    Logger::Debug("Starting update...");

    auto root = fs::absolute(fs::current_path());

    auto temp = (root / "PotatoAlert_temp.exe").string();
    remove(temp.c_str()); // ignore return code

    auto src = (root / "PotatoAlert_new.exe").string();
    auto dst = (root / "PotatoAlert.exe").string();

    rename(dst.c_str(), temp.c_str());
    CopyFile(src.c_str(), dst.c_str(), false);
    static char buffer[512];
    strcpy(buffer, dst.c_str());  // DEPRECATED

    /* CreateProcess API initialization */
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
}

void Updater::download()
{
    // connect(reply, &QNetworkReply::downloadProgress, progressDialog, &ProgressDialog::networkReplyProgress);
}
