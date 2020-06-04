// Copyright 2020 <github.com/razaqq>

#include <QApplication>
#include <QMainWindow>
#include <QWindow>
#include <QFile>
#include <QFont>
#include <QString>
#include <QLatin1String>
#include <iostream>
#include <string>
#include "gui/MainWindow.h"
#include "gui/NativeWindow.h"
#include "utils/Config.h"
#include "utils/Logger.h"
#include "utils/PotatoClient.h"
#include "utils/Palette.h"
#include "utils/Updater.h"


using PotatoAlert::MainWindow;
using PotatoAlert::NativeWindow;
using PotatoAlert::Logger;
using PotatoAlert::Config;
using PotatoAlert::Updater;

int main(int argc, char *argv[]) {
	Q_INIT_RESOURCE(PotatoAlert);

	QApplication app(argc, argv);
	app.setApplicationName("PotatoAlert");
	QFont font = app.font();
	font.setStyleStrategy(QFont::PreferAntialias);
	app.setFont(font);

	Logger l;
	Config c(&l);

	PotatoAlert::PotatoClient* client = new PotatoAlert::PotatoClient(&c, &l);

	MainWindow* mainWindow = new PotatoAlert::MainWindow(&c, &l, client);
	NativeWindow* nativeWindow = new NativeWindow(mainWindow, &c);
	nativeWindow->show();

	QFile file(":/style.qss");
	file.open(QFile::ReadOnly | QFile::Text);
	QString style = QLatin1String(file.readAll());
	app.setStyle("fusion");
	app.setPalette(dark());
	app.setStyleSheet(style);

	if (Updater::updateAvailable())
	{
		if (nativeWindow->confirmUpdate())
			Updater::update();
	}

	int exitCode = app.exec();
	c.save();
	return exitCode;
}
