// Copyright 2021 <github.com/razaqq>

#include "Client/AppDirectories.hpp"
#include "Client/Config.hpp"
#include "Client/ServiceProvider.hpp"

#include "Core/Log.hpp"
#include "Core/StandardPaths.hpp"

#include "Gui/Palette.hpp"
#include "Gui/Updater.hpp"

#include "Updater/Updater.hpp"

#include <QApplication>
#include <QFile>


using PotatoAlert::Client::AppDirectories;
using PotatoAlert::Client::Config;
using PotatoAlert::Client::ServiceProvider;
using PotatoAlert::Updater::Updater;

static int RunMain(int argc, char* argv[])
{
	Q_INIT_RESOURCE(PotatoUpdater);

	AppDirectories appDirs("PotatoAlert");

	PotatoAlert::Core::Log::Init(appDirs.LogFile);
	
	QApplication app(argc, argv);

	if (QApplication::arguments().contains("--clear"))
	{
		Updater::RemoveTrash();
		Updater::StartMain("--changelog");
		return 0;
	}

	QFile file(":/style.qss");
	file.open(QFile::ReadOnly | QFile::Text);
	QString style = QLatin1String(file.readAll());
	QApplication::setStyle("fusion");
	QApplication::setPalette(PotatoAlert::Gui::DarkPalette());
	app.setStyleSheet(style);

	ServiceProvider serviceProvider;
	
	Config config(appDirs.ConfigFile);
	serviceProvider.Add(config);

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
