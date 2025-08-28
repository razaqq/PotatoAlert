// Copyright 2021 <github.com/razaqq>

#include "Client/AppDirectories.hpp"
#include "Client/Config.hpp"
#include "Client/Log.hpp"
#include "Client/ServiceProvider.hpp"
#include "Client/Updater.hpp"

#include "Core/Defer.hpp"
#include "Core/Log.hpp"

#include "Gui/Palette.hpp"
#include "Gui/Updater.hpp"

#include <QApplication>
#include <QFile>


using PotatoAlert::Client::AppDirectories;
using PotatoAlert::Client::Config;
using PotatoAlert::Client::ConfigManager;
using PotatoAlert::Client::ConfigResult;
using PotatoAlert::Client::LogQtMessage;
using PotatoAlert::Client::ServiceProvider;
using PotatoAlert::Client::Updater;

static int RunMain(int argc, char* argv[])
{
	Q_INIT_RESOURCE(PotatoUpdater);

	const AppDirectories appDirs("PotatoAlert");

	PotatoAlert::Core::Log::Init(appDirs.AppDir / "PotatoUpdater.log");
	qInstallMessageHandler(LogQtMessage);
	
	QApplication app(argc, argv);

	if (QApplication::arguments().contains("--clear"))
	{
		Updater::RemoveTrash();
		Updater::StartMain("--changelog");
		return 0;
	}

	QFile file(":/style.qss");
	file.open(QFile::ReadOnly | QFile::Text);
	const QString style = QLatin1String(file.readAll());
	QApplication::setStyle("fusion");
	QApplication::setPalette(PotatoAlert::Gui::DarkPalette());
	app.setStyleSheet(style);

	ServiceProvider serviceProvider;
	
	ConfigManager configManager(appDirs.ConfigFile);
	PA_DEFER
	{
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

	Updater updater;
	serviceProvider.Add(updater);

	PotatoAlert::Gui::Updater gui(serviceProvider);
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
