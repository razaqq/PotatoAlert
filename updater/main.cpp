#include <QApplication>
#include <filesystem>
#include <zip.h>
#include <Palette.hpp>
#include <cstdlib>
#include "Logger.hpp"
#include "UpdaterGui.hpp"


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
		Updater::removeTrash();
		Updater::createProcess(Updater::mainBinary, "--changelog");
		return 0;
	}

	QFile file(":/style.qss");
	file.open(QFile::ReadOnly | QFile::Text);
	QString style = QLatin1String(file.readAll());
	QApplication::setStyle("fusion");
	QApplication::setPalette(PotatoAlert::dark());
	app.setStyleSheet(style);

	Updater u;
	new UpdaterGui(&u);
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