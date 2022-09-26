// Copyright 2021 <github.com/razaqq>

#include "Core/ApplicationGuard.hpp"
#include "Core/Directory.hpp"
#include "Core/StandardPaths.hpp"

#include "Client/AppDirectories.hpp"
#include "Client/Config.hpp"
#include "Client/MatchHistory.hpp"
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
using PotatoAlert::Client::MatchHistory;
using PotatoAlert::Client::PotatoClient;
using PotatoAlert::Client::ReplayAnalyzer;
using PotatoAlert::Client::ServiceProvider;
using PotatoAlert::Core::ApplicationGuard;
using PotatoAlert::Core::AppDataPath;
using PotatoAlert::Gui::DarkPalette;
using PotatoAlert::Gui::LanguageChangeEvent;
using PotatoAlert::Gui::MainWindow;
using PotatoAlert::Gui::NativeWindow;
using PotatoAlert::Updater::Updater;

static int RunMain(int argc, char* argv[])
{
	const ApplicationGuard guard("PotatoAlert");
	if (guard.OtherInstance())
	{
		NativeWindow::RequestFocus();
		ExitProcess(0);
	}

	Q_INIT_RESOURCE(PotatoAlert);

	QApplication app(argc, argv);

	ServiceProvider serviceProvider;

	AppDirectories appDirs("PotatoAlert");
	serviceProvider.Add(appDirs);

	PotatoAlert::Core::Log::Init(appDirs.LogFile);

	Config config(appDirs.ConfigFile);
	serviceProvider.Add(config);

	ReplayAnalyzer replayAnalyzer(serviceProvider, {
		PotatoAlert::Core::GetModuleRootPath().value() / "ReplayVersions",
		appDirs.ReplayVersionsDir
	});
	serviceProvider.Add(replayAnalyzer);

	PotatoClient client(serviceProvider);
	serviceProvider.Add(client);

	MatchHistory matchHistory(serviceProvider);
	serviceProvider.Add(matchHistory);

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
					ExitProcess(0);

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
