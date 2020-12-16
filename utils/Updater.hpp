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
	bool download(const std::string& targetFile);
signals:
	void errorOccurred(QString& text);
	void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
};

}  // namespace PotatoAlert
