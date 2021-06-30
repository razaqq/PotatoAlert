#include "Logger.hpp"
#include "UpdaterGui.hpp"
#include "Updater.hpp"
#include <QApplication>
#include <filesystem>
#include <zip.h>
#include <Palette.hpp>
#include "win32.h"


namespace fs = std::filesystem;
using PotatoAlert::Logger;
using PotatoUpdater::Updater;
using PotatoUpdater::UpdaterGui;


int runMain(int argc, char* argv[])
{
	Q_INIT_RESOURCE(PotatoAlert);
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
	QApplication::setPalette(PotatoAlert::DarkPalette());
	app.setStyleSheet(style);

	new UpdaterGui();
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