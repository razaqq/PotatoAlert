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
		exit(0);
	}

	Q_INIT_RESOURCE(PotatoAlert);

	QApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
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

	return QApplication::exec();
}
