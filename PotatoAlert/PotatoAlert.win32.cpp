// Copyright 2021 <github.com/razaqq>

#include "Core/ApplicationGuard.hpp"

#include "Client/Config.hpp"

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


using PotatoAlert::Core::ApplicationGuard;
using PotatoAlert::Core::PotatoConfig;
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

	if (qgetenv("QT_FONT_DPI").isEmpty())
	{
		qputenv("QT_FONT_DPI", "96");
	}

	PotatoAlert::Core::Log::Init();
	
	QApplication app(argc, argv);

	QApplication::setQuitOnLastWindowClosed(false);
	
	QApplication::setOrganizationName(PRODUCT_COMPANY_NAME);
	QApplication::setApplicationVersion(PRODUCT_VERSION_FULL_STR);

	QFile file(":/style.qss");
	file.open(QFile::ReadOnly | QFile::Text);
	const QString style = QLatin1String(file.readAll());
	QApplication::setStyle("fusion");
	QApplication::setPalette(DarkPalette());
	app.setStyleSheet(style);

	auto mainWindow = new MainWindow();
	auto nativeWindow = new NativeWindow(mainWindow);
	nativeWindow->show();

	// force update of language
	LanguageChangeEvent event(serviceProvider.Get<Config>().Get<ConfigKey::Language>());
	QApplication::sendEvent(mainWindow, &event);

	// check if there is a new version available
	if (PotatoConfig().Get<PotatoAlert::Core::ConfigKey::UpdateNotifications>())
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
