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
#include "Log.hpp"
#include "Palette.hpp"
#include "updater/Updater.hpp"
#include "MainWindow.hpp"
#include "NativeWindow.hpp"
#include "VersionInfo.h"
#include "FramelessWindowsManager.hpp"
#include "win32.h"


using PotatoAlert::MainWindow;
using PotatoAlert::NativeWindow;
using PotatoAlert::PotatoClient;
using PotatoAlert::Logger;
using PotatoAlert::Config;
using PotatoUpdater::Updater;
using PotatoAlert::PotatoConfig;

int runMain(int argc, char* argv[])
{
	Q_INIT_RESOURCE(PotatoAlert);

	PotatoAlert::Log::Init();

	QApplication::setOrganizationName(PRODUCT_COMPANY_NAME);
	QApplication::setApplicationVersion(PRODUCT_VERSION_FULL_STR);

	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

	QApplication app(argc, argv);

	QFont font = QApplication::font();
	font.setStyleStrategy(QFont::PreferAntialias);
	QApplication::setFont(font);

	auto mainWindow = new MainWindow();
	auto nativeWindow = new NativeWindow(mainWindow);
	nativeWindow->show();

	QFile file(":/style.qss");
	file.open(QFile::ReadOnly | QFile::Text);
	QString style = QLatin1String(file.readAll());
	QApplication::setStyle("fusion");
	QApplication::setPalette(PotatoAlert::DarkPalette());
	app.setStyleSheet(style);

	// force update of language
	QEvent event(QEvent::LanguageChange);
	QApplication::sendEvent(mainWindow, &event);

	// check if there is a new version available
	if (PotatoConfig().Get<bool>("update_notifications"))
		if (Updater::UpdateAvailable())
			if (mainWindow->ConfirmUpdate())
				if (Updater::StartUpdater())
					ExitProcess(0);

	if (QApplication::arguments().contains("--changelog"))
		;  // TODO: add changelog window

	return QApplication::exec();
}

#ifndef NDEBUG
int main(int argc, char* argv[])
{
	return runMain(argc, argv);
}
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	return runMain(__argc, __argv);
}
#endif
