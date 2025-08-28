// Copyright 2025 <github.com/razaqq>

#include "Core/ApplicationGuard.hpp"
#include "Core/Defer.hpp"
#include "Core/Directory.hpp"
#include "Core/Process.hpp"
#include "Core/StandardPaths.hpp"
#include "Core/Sqlite.hpp"

#include "Client/AppDirectories.hpp"
#include "Client/Config.hpp"
#include "Client/DatabaseManager.hpp"
#include "Client/FontLoader.hpp"
#include "Client/Log.hpp"
#include "Client/PotatoClient.hpp"
#include "Client/Updater.hpp"

#include "Gui/Palette.hpp"
#include "Gui/NativeWindow.hpp"
#include "Gui/Events.hpp"
#include "Gui/MainWindow.hpp"

#include "VersionInfo.hpp"

#include <QApplication>
#include <QEvent>
#include <QFile>

#ifdef WIN32
	#include "win32.h"
#endif


using PotatoAlert::Client::AppDirectories;
using PotatoAlert::Client::Config;
using PotatoAlert::Client::ConfigManager;
using PotatoAlert::Client::ConfigResult;
using PotatoAlert::Client::DatabaseManager;
using PotatoAlert::Client::LogQtMessage;
using PotatoAlert::Client::LoadFonts;
using PotatoAlert::Client::PotatoClient;
using PotatoAlert::Client::ReplayAnalyzer;
using PotatoAlert::Client::ServiceProvider;
using PotatoAlert::Client::Updater;
using PotatoAlert::Core::ApplicationGuard;
using PotatoAlert::Core::ExitCurrentProcess;
using PotatoAlert::Core::ExitCurrentProcessWithError;
using PotatoAlert::Core::Log;
using PotatoAlert::Core::SQLite;
using PotatoAlert::Gui::DarkPalette;
using PotatoAlert::Gui::FontScalingChangeEvent;
using PotatoAlert::Gui::LanguageChangeEvent;
using PotatoAlert::Gui::MainWindow;
using PotatoAlert::Gui::NativeWindow;


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

	AppDirectories appDirs("PotatoAlert");
	serviceProvider.Add(appDirs);

	Log::Init(appDirs.LogFile);
	qInstallMessageHandler(LogQtMessage);

	ConfigManager configManager(appDirs.ConfigFile);
	PA_DEFER {
		const ConfigResult<void> res = configManager.Save();
		if (!res)
		{
			LOG_ERROR("Failed to save config: {}", res.error());
		}
	};
	const ConfigResult<void> res = configManager.Init();
	if (!res)
	{
		LOG_ERROR("Failed to initialize config: {}", res.error());
		return 1;
	}
	serviceProvider.Add(configManager);

	ReplayAnalyzer replayAnalyzer(serviceProvider);
	serviceProvider.Add(replayAnalyzer);

	PotatoClient client(
	{
		.SubmitUrl = PA_SUBMIT_URL,
		.LookupUrl = PA_LOOKUP_URL,
		.TransferTimeout = 10000,
	}, serviceProvider);
	serviceProvider.Add(client);

	SQLite db = SQLite::Open(appDirs.DatabaseFile, SQLite::Flags::ReadWrite | SQLite::Flags::Create);
	if (!db)
	{
		LOG_ERROR("Failed to open database: {}", db.GetLastError());
		ExitCurrentProcessWithError(1);
	}

	DatabaseManager dbm(db);
	serviceProvider.Add(dbm);

	QApplication::setQuitOnLastWindowClosed(false);

	QApplication::setOrganizationName(PRODUCT_COMPANY_NAME);
	QApplication::setApplicationVersion(PRODUCT_VERSION_FULL_STR);

	QFile file(":/style.qss");
	file.open(QFile::ReadOnly | QFile::Text);
	const QString style = QLatin1String(file.readAll());
	QApplication::setStyle("fusion");
	QApplication::setPalette(DarkPalette());
	app.setStyleSheet(style);

	LoadFonts();
	QFont font(QString::fromStdString(configManager.GetConfig().Font), 9);
	font.setLetterSpacing(QFont::PercentageSpacing, 0);
	font.setStyleStrategy(QFont::PreferAntialias);
	QApplication::setFont(font);

	MainWindow* mainWindow = new MainWindow(serviceProvider);
	NativeWindow* nativeWindow = new NativeWindow(serviceProvider, mainWindow);

	nativeWindow->show();

	// force update of language
	LanguageChangeEvent languageChangeEvent(configManager.GetConfig().Language);
	QApplication::sendEvent(mainWindow, &languageChangeEvent);

	// force update of font scaling
	FontScalingChangeEvent fontScalingChangeEvent(static_cast<float>(configManager.GetConfig().FontScaling) / 100.0f);
	QApplication::sendEvent(mainWindow, &fontScalingChangeEvent);

#ifdef WIN32
	// check if there is a new version available
	if (configManager.GetConfig().UpdateNotifications)
		if (Updater::UpdateAvailable())
			if (mainWindow->ConfirmUpdate())
				if (Updater::StartUpdater())
					ExitCurrentProcess(0);

	if (QApplication::arguments().contains("--changelog"))
		;  // TODO: add changelog
#endif

	return QApplication::exec();
}

#if !defined(NDEBUG) || !defined(WIN32)
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
