// Copyright 2020 <github.com/razaqq>

#include "Client/Config.hpp"

#include "framelesswindowsmanager.h"

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


#include <QDebug>

FRAMELESSHELPER_USE_NAMESPACE

using PotatoAlert::Core::PotatoConfig;
using PotatoAlert::Gui::NativeWindow;

NativeWindow::NativeWindow(QMainWindow* mainWindow) : QWidget(), m_mainWindow(mainWindow)
{
	createWinId();
	this->m_mainWindow->setParent(this);
	this->Init();
}

void NativeWindow::showEvent(QShowEvent* event)
{
	static bool initialized = false;
	if (!initialized)
	{
		QWindow* w = windowHandle();
		FramelessWindowsManager::addWindow(w);
		FramelessWindowsManager::setResizeBorderThickness(w, 4);
		for (auto& o : this->m_titleBar->GetIgnores())
			FramelessWindowsManager::setHitTestVisible(w, o, true);
		FramelessWindowsManager::setTitleBarHeight(w, this->m_titleBar->height());
		initialized = true;
	}
	QWidget::showEvent(event);
}

void NativeWindow::closeEvent(QCloseEvent* event)
{
	if (PotatoConfig().Get<Core::ConfigKey::MinimizeTray>())
	{
		hide();
	}
	else
	{
		PotatoConfig().Set<Core::ConfigKey::WindowHeight>(this->height());
		PotatoConfig().Set<Core::ConfigKey::WindowWidth>(this->width());
		PotatoConfig().Set<Core::ConfigKey::WindowX>(this->windowHandle()->position().x());
		PotatoConfig().Set<Core::ConfigKey::WindowY>(this->windowHandle()->position().y());
		QWidget::closeEvent(event);
		QApplication::exit(0);
	}
}

void NativeWindow::Init()
{
	if (QSystemTrayIcon::isSystemTrayAvailable())
	{
		auto trayIcon = new QSystemTrayIcon(QIcon(":/potato.png"));
		trayIcon->setToolTip("PotatoAlert");

		connect(trayIcon, &QSystemTrayIcon::activated, [this](QSystemTrayIcon::ActivationReason reason)
		{
			if (reason == QSystemTrayIcon::DoubleClick  || reason == QSystemTrayIcon::Trigger)
			{
				this->show();
			}
		});

		auto trayMenu = new QMenu(this);
		auto closeAction = new QAction("Exit", this);
		auto openAction = new QAction("Open", this);
		trayMenu->addAction(openAction);
		trayMenu->addAction(closeAction);
		trayMenu->setLayoutDirection(Qt::LayoutDirection::RightToLeft);
		trayIcon->setContextMenu(trayMenu);

		connect(openAction, &QAction::triggered, this, &QWidget::show);
		connect(closeAction, &QAction::triggered, []()
		{
			QApplication::exit(0);
		});

		trayIcon->show();
	}

	this->m_titleBar->setFixedHeight(23);

	auto layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);

	layout->addWidget(this->m_titleBar);
	layout->addWidget(this->m_mainWindow);

	this->setLayout(layout);

	this->resize(PotatoConfig().Get<Core::ConfigKey::WindowWidth>(), PotatoConfig().Get<Core::ConfigKey::WindowHeight>());
	this->windowHandle()->setPosition(PotatoConfig().Get<Core::ConfigKey::WindowX>(), PotatoConfig().Get<Core::ConfigKey::WindowY>());

	bool reachable = false;
	const QRect windowGeo = windowHandle()->frameGeometry();

	for (const QScreen* screen : QApplication::screens())
	{
		if (windowGeo.intersects(screen->availableGeometry()))
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
