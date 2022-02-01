// Copyright 2020 <github.com/razaqq>

#include "NativeWindow.hpp"

#include "Core/Config.hpp"
#include "FramelessHelper/FramelessWindowsManager.hpp"
#include "TitleBar.hpp"

#include <QApplication>
#include <QMainWindow>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QVBoxLayout>
#include <QWidget>
#include <QWindow>


using PotatoAlert::Core::PotatoConfig;
using PotatoAlert::Gui::NativeWindow;

NativeWindow::NativeWindow(QMainWindow* mainWindow) : QWidget(), m_mainWindow(mainWindow)
{
	this->m_mainWindow->setParent(this);
	this->Init();
}

void NativeWindow::closeEvent(QCloseEvent* event)
{
	if (PotatoConfig().Get<bool>("minimize_tray"))
	{
		hide();
	}
	else
	{
		PotatoConfig().Set<int>("window_height", this->height());
		PotatoConfig().Set<int>("window_width", this->width());
		PotatoConfig().Set<int>("window_x", this->windowHandle()->position().x());
		PotatoConfig().Set<int>("window_y", this->windowHandle()->position().y());
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
			if (reason == QSystemTrayIcon::DoubleClick)
				this->show();
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


	this->createWinId();
	QWindow* w = this->windowHandle();

	this->m_titleBar->setFixedHeight(23);

	for (auto& o : this->m_titleBar->GetIgnores())
		FramelessWindowsManager::addIgnoreObject(w, o);

	FramelessWindowsManager::addWindow(w);
	FramelessWindowsManager::setBorderWidth(w, m_borderWidth);
	FramelessWindowsManager::setBorderHeight(w, m_borderWidth);
	FramelessWindowsManager::setTitleBarHeight(w, this->m_titleBar->height());

	auto layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);

	layout->addWidget(this->m_titleBar);
	layout->addWidget(this->m_mainWindow);

	this->setLayout(layout);

	this->resize(PotatoConfig().Get<int>("window_width"), PotatoConfig().Get<int>("window_height"));
	this->windowHandle()->setPosition(PotatoConfig().Get<int>("window_x"), PotatoConfig().Get<int>("window_y"));
}
