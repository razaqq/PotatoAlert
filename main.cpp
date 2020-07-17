// Copyright 2020 <github.com/razaqq>

#include <QApplication>
#include <QMainWindow>
#include <QWindow>
#include <QFile>
#include <QFont>
#include <QString>
#include <QLatin1String>
#include <string>
#include "utils/Logger.h"
#include "utils/Config.h"
#include "utils/PotatoClient.h"
#include "utils/Palette.h"
#include "utils/Updater.h"
#include "gui/MainWindow.h"
#include "gui/NativeWindow.h"
#include "VersionInfo.h"
#include <iostream>
#include "StringTable.h"


using PotatoAlert::MainWindow;
using PotatoAlert::NativeWindow;
using PotatoAlert::PotatoClient;
using PotatoAlert::Logger;
using PotatoAlert::Config;
using PotatoAlert::Updater;
using PotatoAlert::PotatoConfig;
using PotatoAlert::PotatoLogger;

int main(int argc, char *argv[]) {
	Q_INIT_RESOURCE(PotatoAlert);

	QApplication app(argc, argv);
    QApplication::setOrganizationName(PRODUCT_COMPANY_NAME);

	std::cout << PotatoAlert::GetString("English", PotatoAlert::Keys::COLUMN_AVERAGE_DAMAGE);

	QFont font = QApplication::font();
	font.setStyleStrategy(QFont::PreferAntialias);
	QApplication::setFont(font);

	PotatoClient client;
	auto mainWindow = new MainWindow(&client);
    auto nativeWindow = new NativeWindow(mainWindow);
    nativeWindow->show();

    QFile file(":/style.qss");
    file.open(QFile::ReadOnly | QFile::Text);
    QString style = QLatin1String(file.readAll());
    QApplication::setStyle("fusion");
    QApplication::setPalette(dark());
    app.setStyleSheet(style);

    if (Updater::updateAvailable())
    {
        if (nativeWindow->confirmUpdate())
            Updater::update();
    }

    int exitCode = QApplication::exec();
    PotatoConfig().save();
    return exitCode;
}
