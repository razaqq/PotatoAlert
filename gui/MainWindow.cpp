// Copyright 2020 <github.com/razaqq>

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
#include "Updater.hpp"
#include "PotatoClient.hpp"
#include "FramelessDialog.hpp"
#include "StatsWidget/StatsWidget.hpp"
#include "StatsWidget/StatsHeader.hpp"
#include "MenuBar/VerticalMenuBar.hpp"
#include "StringTable.hpp"
#include "MainWindow.hpp"


using PotatoAlert::MainWindow;

MainWindow::MainWindow(PotatoClient* pc) : QMainWindow()
{
	this->pc = pc;
	this->init();
	this->connectSignals();
	this->pc->init();
}

void MainWindow::init()
{
	// central widget
	this->setCentralWidget(this->centralW);

	this->layout()->setContentsMargins(0, 0, 0, 0);
	this->layout()->setSpacing(0);
	this->centralLayout->setContentsMargins(0, 0, 0, 0);
	this->centralLayout->setSpacing(0);
	this->centralW->setLayout(centralLayout);

	// menubar dock widget
	const bool leftSide = PotatoConfig().get<bool>("menubar_leftside");
	const auto side = leftSide ? Qt::DockWidgetArea::LeftDockWidgetArea : Qt::DockWidgetArea::RightDockWidgetArea;
	this->addDockWidget(side, this->menuBar);
	connect(this->menuBar, &VerticalMenuBar::dockLocationChanged, [](Qt::DockWidgetArea area)
	{
		PotatoConfig().set<bool>("menubar_leftside", area == Qt::DockWidgetArea::LeftDockWidgetArea);
	});

	this->settingsWidget = new SettingsWidget(this, this->pc);

	// set other tabs invisible
	this->settingsWidget->setVisible(false);
	this->aboutWidget->setVisible(false);

	this->centralLayout->addWidget(this->statsWidget);
	this->centralLayout->addWidget(this->settingsWidget);
	this->centralLayout->addWidget(this->aboutWidget);
}

void MainWindow::switchTab(int i)
{
	QWidget* oldWidget = this->activeWidget;
	switch (i)
	{
	case 0:  // stats table
		this->activeWidget = this->statsWidget;
		break;
	case 1:  // settings
		this->activeWidget = this->settingsWidget;
		break;
	case 2:  // discord
		QDesktopServices::openUrl(QUrl("https://discord.gg/Ut8t8PA"));
		return;
		case 3:  // log
		QDesktopServices::openUrl(QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/PotatoAlert"));
		return;
		case 4:  // github
		QDesktopServices::openUrl(QUrl("https://github.com/razaqq/PotatoAlert"));
		return;
	case 5:  // about
		this->activeWidget = this->aboutWidget;
		break;
	default:
		break;
	}
	oldWidget->setVisible(false);
	this->activeWidget->setVisible(true);
}

void MainWindow::connectSignals()
{
	connect(this->menuBar->btnGroup, &QButtonGroup::idClicked, this, &MainWindow::switchTab);

	connect(this->pc, &PotatoClient::status, this->statsWidget, &StatsWidget::setStatus);
	connect(this->pc, &PotatoClient::teamsReady, this->statsWidget, &StatsWidget::fillTables);
	connect(this->pc, &PotatoClient::avgReady, this->statsWidget, &StatsWidget::setAverages);
	connect(this->pc, &PotatoClient::clansReady, this->statsWidget, &StatsWidget::setClans);
	connect(this->pc, &PotatoClient::wowsNumbersReady, this->statsWidget, &StatsWidget::setWowsNumbers);

	connect(this->settingsWidget, &SettingsWidget::done,[this]()
	{
		this->switchTab(0);
		this->menuBar->btnGroup->button(0)->setChecked(true);
	});
}

int MainWindow::confirmUpdate()
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

void MainWindow::startUpdate(Updater* updater)
{
	auto dialog = new FramelessDialog(this);

	auto vLayout = new QVBoxLayout();

	auto waitLabel = new QLabel(PotatoAlert::GetString(PotatoAlert::StringKeys::UPDATE_DOWNLOADING));

	auto progressBar = new QProgressBar();
	progressBar->setValue(0);
	progressBar->setMaximum(100);

	auto progressLayout = new QHBoxLayout();
	auto progressLabel = new QLabel();
	auto speedLabel = new QLabel();
	progressLayout->addStretch();
	progressLayout->addWidget(progressLabel);
	progressLayout->addStretch();
	progressLayout->addWidget(speedLabel);
	progressLayout->addStretch();

	vLayout->addWidget(waitLabel, 0, Qt::AlignHCenter);
	vLayout->addWidget(progressBar);
	vLayout->addLayout(progressLayout);
	dialog->setLayout(vLayout);

	connect(updater, &Updater::downloadProgress,
	[progressBar, progressLabel, speedLabel](int percent, QString& progress, QString& speed)
	{
		progressBar->setValue(percent);
		progressLabel->setText(progress);
		speedLabel->setText(speed);
	});

	connect(updater, &Updater::errorOccurred, [dialog](QString& text)
	{
		// clear dialog from progress
		qDeleteAll(dialog->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly));
		delete dialog->layout();

		auto errorLabel = new QLabel(PotatoAlert::GetString(PotatoAlert::StringKeys::UPDATE_FAILED) + text);
		errorLabel->setWordWrap(true);

		auto errorIcon = new QLabel();
		auto errorPix = QPixmap(":/error.png").scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		errorIcon->setPixmap(errorPix);

		auto hLayout = new QHBoxLayout();
		hLayout->addStretch();
		hLayout->addWidget(errorIcon);
		hLayout->addStretch();
		hLayout->addWidget(errorLabel);
		hLayout->addStretch();

		auto okButton = new QPushButton(PotatoAlert::GetString(PotatoAlert::StringKeys::OK));
		okButton->setObjectName("confirmButton");
		connect(okButton, &QPushButton::clicked, [dialog]() { dialog->close(); });

		auto vLayout = new QVBoxLayout();
		vLayout->addLayout(hLayout);
		vLayout->addWidget(okButton);

		dialog->setLayout(vLayout);
	});

	dialog->exec();
}
