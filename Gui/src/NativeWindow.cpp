// Copyright 2020 <github.com/razaqq>

#include "Client/Config.hpp"
#include "Client/ServiceProvider.hpp"

#include <FramelessWidgetsHelper>

#include "Gui/NativeWindow.hpp"
#include "Gui/TitleBar.hpp"

#include <QApplication>
#include <QMainWindow>
#include <QMenu>
#include <QScreen>
#include <QSystemTrayIcon>
#include <QVBoxLayout>
#include <QWidget>
#include <QWindow>


using wangwenx190::FramelessHelper::FramelessWidgetsHelper;
using wangwenx190::FramelessHelper::Global::SystemButtonType;
using PotatoAlert::Client::Config;
using PotatoAlert::Client::ConfigKey;
using PotatoAlert::Gui::NativeWindow;

NativeWindow::NativeWindow(const Client::ServiceProvider& serviceProvider, QMainWindow* mainWindow, QWidget* parent) : QWidget(parent), m_services(serviceProvider), m_mainWindow(mainWindow)
{
	createWinId();
	m_mainWindow->setParent(this);
	Init();

	FramelessWidgetsHelper* helper = FramelessWidgetsHelper::get(this);

	helper->extendsContentIntoTitleBar();
	helper->setTitleBarWidget(m_titleBar);

	helper->setSystemButton(m_titleBar->GetMaximizeButton(), SystemButtonType::Maximize);
	helper->setSystemButton(m_titleBar->GetMinimizeButton(), SystemButtonType::Minimize);
	helper->setSystemButton(m_titleBar->GetCloseButton(), SystemButtonType::Close);
	helper->setSystemButton(m_titleBar->GetRestoreButton(), SystemButtonType::Restore);

	helper->setHitTestVisible(m_titleBar->GetMaximizeButton(), true);
	helper->setHitTestVisible(m_titleBar->GetMinimizeButton(), true);
	helper->setHitTestVisible(m_titleBar->GetCloseButton(), true);
	helper->setHitTestVisible(m_titleBar->GetRestoreButton(), true);
}

void NativeWindow::hideEvent(QHideEvent* event)
{
	Config& config = m_services.Get<Config>();

	// ignore minimize events
	if (isMinimized())
	{
		return;
	}

	if (config.Get<ConfigKey::MinimizeTray>())
	{
		hide();
	}
	else
	{
		// if window is not maximized, save geometry
		if ((windowState() & Qt::WindowMaximized) == 0)
		{
			config.Set<ConfigKey::WindowHeight>(height());
			config.Set<ConfigKey::WindowWidth>(width());
			config.Set<ConfigKey::WindowX>(windowHandle()->framePosition().x());
			config.Set<ConfigKey::WindowY>(windowHandle()->framePosition().y());
		}
		config.Set<ConfigKey::WindowState>(windowState());

		QWidget::hideEvent(event);
		QApplication::exit(0);
	}
}

void NativeWindow::showEvent(QShowEvent* event)
{
	const Config& config = m_services.Get<Config>();

	setGeometry(
		config.Get<ConfigKey::WindowX>(),
		config.Get<ConfigKey::WindowY>(),
		config.Get<ConfigKey::WindowWidth>(),
		config.Get<ConfigKey::WindowHeight>()
	);
	setWindowState(static_cast<decltype(windowState())>(config.Get<ConfigKey::WindowState>()));
}

void NativeWindow::Init()
{
	if (QSystemTrayIcon::isSystemTrayAvailable())
	{
		QSystemTrayIcon* trayIcon = new QSystemTrayIcon(QIcon(":/potato.svg"));
		trayIcon->setToolTip("PotatoAlert");

		connect(trayIcon, &QSystemTrayIcon::activated, [this](QSystemTrayIcon::ActivationReason reason)
		{
			if (reason == QSystemTrayIcon::DoubleClick || reason == QSystemTrayIcon::Trigger)
			{
				show();
			}
		});

		QMenu* trayMenu = new QMenu(this);
		QAction* closeAction = new QAction("Exit", this);
		QAction* openAction = new QAction("Open", this);
		trayMenu->addAction(openAction);
		trayMenu->addAction(closeAction);
		trayMenu->setLayoutDirection(Qt::LayoutDirection::RightToLeft);
		trayIcon->setContextMenu(trayMenu);

		connect(openAction, &QAction::triggered, this, &NativeWindow::show);
		connect(closeAction, &QAction::triggered, []()
		{
			QApplication::exit(0);
		});

		trayIcon->show();
	}

	m_titleBar->setFixedHeight(23);

	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);

	layout->addWidget(m_titleBar);
	layout->addWidget(m_mainWindow);

	setLayout(layout);

	bool reachable = false;
	QRect titleBarGeo = windowHandle()->frameGeometry();
	titleBarGeo.setBottom(titleBarGeo.top() + m_titleBar->height());

	for (const QScreen* screen : QApplication::screens())
	{
		if (titleBarGeo.intersects(screen->availableGeometry()))
		{
			reachable = true;
			break;
		}
	}

	if (!reachable)
	{
		windowHandle()->setPosition(100, 100);
	}
}
