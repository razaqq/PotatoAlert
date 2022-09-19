// Copyright 2020 <github.com/razaqq>

#include "Client/Config.hpp"

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
using PotatoAlert::Core::PotatoConfig;
using PotatoAlert::Gui::NativeWindow;

NativeWindow::NativeWindow(QMainWindow* mainWindow, QWidget* parent) : QWidget(parent), m_mainWindow(mainWindow)
{
	createWinId();
	m_mainWindow->setParent(this);
	Init();

	QWindow* w = windowHandle();
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
	if (PotatoConfig().Get<Core::ConfigKey::MinimizeTray>())
	{
		hide();
	}
	else
	{
		if ((windowState() & Qt::WindowMaximized) == 0)
		{
			PotatoConfig().Set<Core::ConfigKey::WindowHeight>(height());
			PotatoConfig().Set<Core::ConfigKey::WindowWidth>(width());
			PotatoConfig().Set<Core::ConfigKey::WindowX>(windowHandle()->framePosition().x());
			PotatoConfig().Set<Core::ConfigKey::WindowY>(windowHandle()->framePosition().y());
		}
		PotatoConfig().Set<Core::ConfigKey::WindowState>(windowState());

		QWidget::hideEvent(event);
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
				show();
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

	m_titleBar->setFixedHeight(23);

	auto layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);

	layout->addWidget(m_titleBar);
	layout->addWidget(m_mainWindow);

	setLayout(layout);
	
	resize(PotatoConfig().Get<Core::ConfigKey::WindowWidth>(), PotatoConfig().Get<Core::ConfigKey::WindowHeight>());
	windowHandle()->setFramePosition(QPoint(PotatoConfig().Get<Core::ConfigKey::WindowX>(), PotatoConfig().Get<Core::ConfigKey::WindowY>()));
	setWindowState(static_cast<decltype(windowState())>(PotatoConfig().Get<Core::ConfigKey::WindowState>()));

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
