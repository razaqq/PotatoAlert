// Copyright 2020 <github.com/razaqq>

#include <QApplication>
#include <QMainWindow>
#include <QWindow>
#include <QFile>
#include <QFont>
#include <QString>
#include <QLatin1String>
#include <string>
#include "Config.hpp"
#include "PotatoClient.hpp"
#include "Palette.hpp"
#include "updater/Updater.hpp"
#include "MainWindow.hpp"
#include "NativeWindow.hpp"
#include "VersionInfo.h"
#include <Windows.h>
#include <shellapi.h>
#include <io.h>
#include <fcntl.h>


using PotatoAlert::MainWindow;
using PotatoAlert::NativeWindow;
using PotatoAlert::PotatoClient;
using PotatoAlert::Logger;
using PotatoAlert::Config;
using PotatoUpdater::Updater;
using PotatoAlert::PotatoConfig;

int runMain(QApplication& app)
{
	QApplication::setOrganizationName(PRODUCT_COMPANY_NAME);
	QApplication::setApplicationVersion(PRODUCT_VERSION_FULL_STR);

	QFont font = QApplication::font();
	font.setStyleStrategy(QFont::PreferAntialias);
	QApplication::setFont(font);

	PotatoClient client;
	auto mainWindow = new MainWindow(&client);
	auto nativeWindow = new NativeWindow(mainWindow);
	nativeWindow->show();

	QFile file(":/style.qss");
	file.open(QFile::ReadOnly | QFile::Text);
	QString style = QLatin1String(file.readAll());
	QApplication::setStyle("fusion");
	QApplication::setPalette(PotatoAlert::dark());
	app.setStyleSheet(style);

	// force update of language
	QEvent event(QEvent::LanguageChange);
	QApplication::sendEvent(mainWindow, &event);

	// check if there is a new version available
	if (PotatoConfig().get<bool>("update_notifications"))
		if (Updater::updateAvailable())
			if (mainWindow->confirmUpdate())
				if (Updater::createProcess())
					ExitProcess(0);

	if (QApplication::arguments().contains("--changelog"))
		;  // TODO: add changelog window

	return QApplication::exec();
}

#ifndef NDEBUG
int main(int argc, char* argv[])
{
	Q_INIT_RESOURCE(PotatoAlert);
	QApplication app(argc, argv);
	return runMain(app);
}
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	Q_INIT_RESOURCE(PotatoAlert);
	QApplication app(__argc, __argv);
	return runMain(app);
}
#endif
