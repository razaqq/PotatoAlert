// Copyright 2021 <github.com/razaqq>

#include "Core/Config.hpp"
#include "Core/Mutex.hpp"
#include "Gui/MainWindow.hpp"
#include "Gui/NativeWindow.hpp"
#include "Gui/Palette.hpp"
#include "Updater/Updater.hpp"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#include "VersionInfo.h"
#pragma clang diagnostic pop

#include "win32.h"

#include <QApplication>
#include <QFile>


using PotatoAlert::Core::PotatoConfig;
using PotatoAlert::Core::Mutex;
using PotatoAlert::Gui::DarkPalette;
using PotatoAlert::Gui::MainWindow;
using PotatoAlert::Gui::NativeWindow;
using PotatoUpdater::Updater;

static int RunMain(int argc, char* argv[])
{
	const std::string mutexName = "PotatoAlert-0D54203D-6BF9-4E96-8CD7-2BE3E780E013";
	if (Mutex::Open(mutexName))
	{
		NativeWindow::RequestFocus();
		_exit(0);
	}
	Mutex::Create(mutexName, true);

	Q_INIT_RESOURCE(PotatoAlert);

	QApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
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

	/*
	QFont font;
	// qt5: family, pointSizeF, pixelSize, styleHint, weight, style, underline, strikeOut, fixedPitch, (int)false, styleName
	// qt6: family, pointSizeF, pixelSize, styleHint, weight, style, underline, strikeOut, fixedPitch, (int)false, capitalization, letterSpacingType, letterSpacing, wordSpacing, stretch, styleStrategy, styleName
	// qt5: "MS Shell Dlg 2, 8.25, -1, 5, 50, 0, 0, 0, 0, 0, 0, 0, 0"
	font.setFamily("MS Shell Dlg 2");
	font.setPointSizeF(8.25);
	font.setPixelSize(-1);
	font.setStyleHint(QFont::AnyStyle);
	font.setWeight(QFont::Weight::Normal);  // qt6=QFont::Weight::Normal, qt5=50
	font.setStyle(QFont::StyleNormal);
	font.setUnderline(false);
	font.setStrikeOut(false);
	font.setFixedPitch(false);
	font.setCapitalization(QFont::MixedCase);
	font.setLetterSpacing(QFont::PercentageSpacing, 0);
	font.setWordSpacing(0);
	font.setStretch(0);
	// font.setStyleStrategy(QFont::PreferAntialias);
	font.setKerning(true);
	app.setFont(font);
	// qDebug() << qApp->font();
	*/

	auto mainWindow = new MainWindow();
	auto nativeWindow = new NativeWindow(mainWindow);
	nativeWindow->show();

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