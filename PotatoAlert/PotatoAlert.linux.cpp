// Copyright 2021 <github.com/razaqq>

#include "Core/ApplicationGuard.hpp"
#include "Core/Defer.hpp"
#include "Core/Semaphore.hpp"

#include "Client/Config.hpp"

#include "Gui/MainWindow.hpp"
#include "Gui/NativeWindow.hpp"
#include "Gui/Palette.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#include "VersionInfo.h"
#pragma clang diagnostic pop

#include <QApplication>
#include <QFile>


using PotatoAlert::Core::ApplicationGuard;
using PotatoAlert::Core::PotatoConfig;
using PotatoAlert::Gui::DarkPalette;
using PotatoAlert::Gui::MainWindow;
using PotatoAlert::Gui::NativeWindow;

int main(int argc, char* argv[])
{
	const ApplicationGuard guard("PotatoAlert");
	if (guard.OtherInstance())
	{
		NativeWindow::RequestFocus();
		std::exit(0);
	}

	wangwenx190::FramelessHelper::FramelessHelper::Core::initialize();

	Q_INIT_RESOURCE(PotatoAlert);

	if (qEnvironmentVariableIsEmpty("QT_FONT_DPI"))
	{
		qputenv("QT_FONT_DPI", "96");
	}

	PotatoAlert::Core::Log::Init();

	QApplication app(argc, argv);

	QApplication::setQuitOnLastWindowClosed(false);
	QApplication::setOrganizationName(PRODUCT_COMPANY_NAME);
	QApplication::setApplicationVersion(PRODUCT_VERSION_FULL_STR);
	QApplication::setWindowIcon(QIcon(":/potato.png"));

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
	QEvent event(QEvent::LanguageChange);
	QApplication::sendEvent(mainWindow, &event);

	return QApplication::exec();
}
