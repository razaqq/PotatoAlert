// Copyright 2020 <github.com/razaqq>
#pragma once

#include <string>
#include <filesystem>
#include <QObject>
#include <QWidget>
#include <QtNetwork>
#include <QNetworkAccessManager>


namespace fs = std::filesystem;

namespace PotatoAlert {

class Updater : public QWidget
{
	Q_OBJECT
public:
	static bool updateAvailable();
	void start();
private:
	// static bool getAdmin();
	// static void restart();
	bool download(const std::string& targetFile, const std::string& dest, const std::string& exeFile);
	static bool unpack(const char* file, const char* dest);
	static void restart(const char* exeFile, const char* args);
signals:
	void errorOccurred(QString& text);
	void downloadProgress(int percent, QString& progress, QString& speed);
};

}  // namespace PotatoAlert
