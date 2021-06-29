// Copyright 2020 <github.com/razaqq>

#include "MainWindow.hpp"
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSizeGrip>
#include <QIcon>
#include <QUrl>
#include <QSettings>
#include <QWindow>
#include <QButtonGroup>
#include <QDesktopServices>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QApplication>
#include <QProgressBar>
#include "Config.hpp"
#include "Logger.hpp"
#include "PotatoClient.hpp"
#include "FramelessDialog.hpp"
#include "StatsWidget/StatsWidget.hpp"
#include "StatsWidget/StatsHeader.hpp"
#include "MenuBar/VerticalMenuBar.hpp"
#include "StringTable.hpp"
#include "CSVWriter.hpp"


using PotatoAlert::MainWindow;

MainWindow::MainWindow(PotatoClient* pc) : QMainWindow()
{
	this->pc = pc;
	this->Init();
	this->ConnectSignals();
	this->pc->Init();
}

void MainWindow::Init()
{
	// central widget
	this->setCentralWidget(this->centralW);

	this->layout()->setContentsMargins(0, 0, 0, 0);
	this->layout()->setSpacing(0);

	this->centralLayout->setContentsMargins(0, 0, 0, 0);
	this->centralLayout->setSpacing(0);
	this->centralW->setLayout(centralLayout);

	// menubar dock widget
	const bool leftSide = PotatoConfig().Get<bool>("menubar_leftside");
	const auto side = leftSide ? Qt::DockWidgetArea::LeftDockWidgetArea : Qt::DockWidgetArea::RightDockWidgetArea;
	this->addDockWidget(side, this->menuBar);
	connect(this->menuBar, &VerticalMenuBar::dockLocationChanged, [](Qt::DockWidgetArea area)
	{
		PotatoConfig().Set<bool>("menubar_leftside",area == Qt::DockWidgetArea::LeftDockWidgetArea);
	});

	this->settingsWidget = new SettingsWidget(this, this->pc);

	// set other tabs invisible
	this->settingsWidget->setVisible(false);
	this->aboutWidget->setVisible(false);

	this->centralLayout->addWidget(this->statsWidget);
	this->centralLayout->addWidget(this->settingsWidget);
	this->centralLayout->addWidget(this->aboutWidget);
}

void MainWindow::SwitchTab(MenuEntry i)
{
	QWidget* oldWidget = this->activeWidget;
	switch (i)
	{
	case MenuEntry::Table:
		this->activeWidget = this->statsWidget;
		break;
	case MenuEntry::Settings:
		this->activeWidget = this->settingsWidget;
		break;
	case MenuEntry::Discord:
		QDesktopServices::openUrl(QUrl("https://discord.gg/Ut8t8PA"));
		return;
	case MenuEntry::CSV:
		QDesktopServices::openUrl(QUrl(CSV::GetDir()));
		return;
	case MenuEntry::Log:
		QDesktopServices::openUrl(QUrl(Logger::GetDir()));
		return;
	case MenuEntry::Github:
		QDesktopServices::openUrl(QUrl("https://github.com/razaqq/PotatoAlert"));
		return;
	case MenuEntry::About:
		this->activeWidget = this->aboutWidget;
		break;
	}
	oldWidget->setVisible(false);
	this->activeWidget->setVisible(true);
}

void MainWindow::ConnectSignals()
{
	connect(this->menuBar, &VerticalMenuBar::EntryClicked, this, &MainWindow::SwitchTab);

	connect(this->pc, &PotatoClient::status, this->statsWidget, &StatsWidget::SetStatus);
	connect(this->pc, &PotatoClient::matchReady, this->statsWidget, &StatsWidget::Update);

	connect(this->settingsWidget, &SettingsWidget::done, [this]()
	{
		this->SwitchTab(MenuEntry::Table);
		this->menuBar->SetChecked(MenuEntry::Table);
	});
}

bool MainWindow::ConfirmUpdate()
{
	auto dialog = new FramelessDialog(this);

	/*
	 * TODO: FIX WARNING
	 * QWindowsWindow::setGeometry: Unable to set geometry
	*/

	auto buttonBox = new QDialogButtonBox();
	buttonBox->setAttribute(Qt::WA_TranslucentBackground);

	auto yesButton = new QPushButton(PotatoAlert::GetString(PotatoAlert::StringKeys::YES), buttonBox);
	yesButton->setObjectName("confirmButton");

	auto noButton = new QPushButton(PotatoAlert::GetString(PotatoAlert::StringKeys::NO), buttonBox);
	noButton->setObjectName("confirmButton");

	connect(yesButton, &QPushButton::clicked, [dialog](int button) { dialog->done(QDialog::Accepted); });
	connect(noButton, &QPushButton::clicked, [dialog](int button) { dialog->done(QDialog::Rejected); });

	buttonBox->addButton(yesButton, QDialogButtonBox::ActionRole);
	buttonBox->addButton(noButton, QDialogButtonBox::ActionRole);
	buttonBox->setCenterButtons(true);

	auto textField = new QLabel(PotatoAlert::GetString(PotatoAlert::StringKeys::UPDATE_QUESTION));
	textField->setWordWrap(true);

	auto icon = new QIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxQuestion));
	auto iconLabel = new QLabel();
	iconLabel->setPixmap(icon->pixmap(100, 100));

	auto layout = new QVBoxLayout();
	layout->setContentsMargins(15, 15, 15, 10);

	auto textLayout = new QHBoxLayout();
	textLayout->addWidget(iconLabel, 0, Qt::AlignRight);
	textLayout->addWidget(textField, 0, Qt::AlignLeft);

	layout->addLayout(textLayout);
	layout->addWidget(buttonBox,0, Qt::AlignHCenter);

	dialog->setLayout(layout);

	dialog->show();

	return dialog->exec();
}
