// Copyright 2020 <github.com/razaqq>

#include "NativeWindow.hpp"

#include "Core/Config.hpp"
#include "FramelessHelper/FramelessWindowsManager.hpp"
#include "TitleBar.hpp"

#include <QMainWindow>
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
	PotatoConfig().Set<int>("window_height", this->height());
	PotatoConfig().Set<int>("window_width", this->width());
	PotatoConfig().Set<int>("window_x", this->windowHandle()->position().x());
	PotatoConfig().Set<int>("window_y", this->windowHandle()->position().y());
	QWidget::closeEvent(event);
}

void NativeWindow::Init()
{
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
