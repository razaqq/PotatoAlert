// Copyright 2021 <github.com/razaqq>

#include "Core/ApplicationGuard.hpp"
#include "Core/Directory.hpp"
#include "Core/Process.hpp"
#include "Core/StandardPaths.hpp"
#include "Core/Sqlite.hpp"

#include "Client/AppDirectories.hpp"
#include "Client/Config.hpp"
#include "Client/DatabaseManager.hpp"
#include "Client/ServiceProvider.hpp"
#include "Client/ReplayAnalyzer.hpp"

#include "Gui/MainWindow.hpp"
#include "Gui/NativeWindow.hpp"
#include "Gui/Palette.hpp"
#include "Gui/LanguageChangeEvent.hpp"

#include "Updater/Updater.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#include "VersionInfo.h"
#pragma clang diagnostic pop

#include "win32.h"

#include <QApplication>
#include <QFile>
#include <QEvent>


using PotatoAlert::Client::AppDirectories;
using PotatoAlert::Client::Config;
using PotatoAlert::Client::ConfigKey;
using PotatoAlert::Client::DatabaseManager;
using PotatoAlert::Client::PotatoClient;
using PotatoAlert::Client::ReplayAnalyzer;
using PotatoAlert::Client::ServiceProvider;
using PotatoAlert::Core::ApplicationGuard;
using PotatoAlert::Core::AppDataPath;
using PotatoAlert::Core::ExitCurrentProcess;
using PotatoAlert::Core::ExitCurrentProcessWithError;
using PotatoAlert::Core::SQLite;
using PotatoAlert::Gui::DarkPalette;
using PotatoAlert::Gui::LanguageChangeEvent;
using PotatoAlert::Gui::MainWindow;
using PotatoAlert::Gui::NativeWindow;
using PotatoAlert::Updater::Updater;

static int RunMain(int argc, char* argv[])
{
	const ApplicationGuard guard("PotatoAlert");
	if (guard.ExistsOtherInstance())
	{
		NativeWindow::RequestFocus();
		ExitCurrentProcess(0);
	}

	Q_INIT_RESOURCE(PotatoAlert);

	QApplication app(argc, argv);

	ServiceProvider serviceProvider;

	// AppDirectories appDirs("PotatoAlertХасанпасданм");
	AppDirectories appDirs("PotatoAlert");
	serviceProvider.Add(appDirs);

	PotatoAlert::Core::Log::Init(appDirs.LogFile);

	Config config(appDirs.ConfigFile);
	serviceProvider.Add(config);

	ReplayAnalyzer replayAnalyzer(serviceProvider, appDirs.ReplayVersionsDir);
	serviceProvider.Add(replayAnalyzer);

	PotatoClient client(serviceProvider);
	serviceProvider.Add(client);

	SQLite db = SQLite::Open(appDirs.DatabaseFile, SQLite::Flags::ReadWrite | SQLite::Flags::Create);
	if (!db)
	{
		LOG_ERROR("Failed to open database: {}", db.GetLastError());
		ExitCurrentProcessWithError(1);
	}

	DatabaseManager dbm(db);
	serviceProvider.Add(dbm);
	auto res = dbm.GetMatches();

	QApplication::setQuitOnLastWindowClosed(false);
	
	QApplication::setOrganizationName(PRODUCT_COMPANY_NAME);
	QApplication::setApplicationVersion(PRODUCT_VERSION_FULL_STR);

	QFile file(":/style.qss");
	file.open(QFile::ReadOnly | QFile::Text);
	const QString style = QLatin1String(file.readAll());
	QApplication::setStyle("fusion");
	QApplication::setPalette(DarkPalette());
	app.setStyleSheet(style);

	auto mainWindow = new MainWindow(serviceProvider);
	auto nativeWindow = new NativeWindow(serviceProvider, mainWindow);

	nativeWindow->show();

	// force update of language
	LanguageChangeEvent event(serviceProvider.Get<Config>().Get<ConfigKey::Language>());
	QApplication::sendEvent(mainWindow, &event);

	// check if there is a new version available
	if (serviceProvider.Get<Config>().Get<ConfigKey::UpdateNotifications>())
		if (Updater::UpdateAvailable())
			if (mainWindow->ConfirmUpdate())
				if (Updater::StartUpdater())
					ExitCurrentProcess(0);

	if (QApplication::arguments().contains("--changelog"))
		;  // TODO: add changelog

	return QApplication::exec();
}

#ifndef NDEBUG
int main(int argc, char* argv[])
{
	return RunMain(argc, argv);
}
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	return RunMain(__argc, __argv);
}
#endif
